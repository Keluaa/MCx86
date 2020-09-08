 
#include <iostream>
#include <bitset>
 
#include "data_types.h"
#include "ALU.h"
#include "registers.h"
#include "RAM.h"
#include "ROM.h"
#include "Instructions.h"

void load_fibbonacci(ISA::Instruction** instructions, const int instructions_count)
{
	if (instructions_count < 6) {
		std::cout << "Not enough space for fibbonacci program.\n";
		return;
	}

	int i = 0;
	instructions[i++] = new ISA::MOV(ISA::Registers::EAX, U8(0));
	instructions[i++] = new ISA::MOV(ISA::Registers::EDX, U8(1));
	instructions[i++] = new ISA::ADD(ISA::Registers::EAX, ISA::Registers::EDX);
	instructions[i++] = new ISA::XCHG(ISA::Registers::EAX, ISA::Registers::EDX);
	instructions[i++] = new ISA::JNO(2);
	instructions[i++] = new ISA::STOP();
}

int main()
{
	Registers registers;
 
	const int instructions_count = 6;

	ISA::Instruction** instructions = new ISA::Instruction*[instructions_count];
	std::memset(instructions, NULL, instructions_count);

	load_fibbonacci(instructions, instructions_count);
	
	const int max_cycles = 1000;
	int cycle = 0;
	while (registers.EIP < instructions_count) {
		cycle++;
		if (cycle >= max_cycles) {
			std::cout << "Max cycles reached. Interrupting program.\n";
			break;
		}
		
		//std::cout << "\nInstruction Pointer: " << registers.EIP << "\n";
		//std::cout << "EAX: " << registers.read(ISA::Registers::EAX) << " (" << std::bitset<32>(registers.read(ISA::Registers::EAX)) << ")\n";
		//std::cout << "EDX: " << registers.read(ISA::Registers::EDX) << " (" << std::bitset<32>(registers.read(ISA::Registers::EDX)) << ")\n";
 
		const ISA::Instruction* base_inst = instructions[registers.EIP];
		//std::cout << "Opcode: " << std::hex << (int) base_inst->opcode() << std::dec << "\n";
		switch (base_inst->opcode())
		{
		case ISA::Opcodes::ADD:
		{
			const ISA::ADD* inst = static_cast<const ISA::ADD*>(base_inst);
			bit carry = 0;
			U32 res = ALU::add(registers.read(inst->destination), registers.read(inst->operand), carry);
			registers.write(inst->destination, res);
			registers.flag_write_CF(carry);
			registers.flag_write_OF(carry);
			break;
		}
		case ISA::Opcodes::MOV:
		{
			const ISA::MOV* inst = static_cast<const ISA::MOV*>(base_inst);
			registers.write(inst->destination, inst->source);
			break;
		}
		case ISA::Opcodes::JO:
		{
			const ISA::JO* inst = static_cast<const ISA::JO*>(base_inst);
			if (registers.flag_read_OF()) {
				registers.EIP = inst->address;
				continue; // skip EIP increment
			}
			break;
		}
		case ISA::Opcodes::JNO:
		{
			const ISA::JNO* inst = static_cast<const ISA::JNO*>(base_inst);
			if (!registers.flag_read_OF()) {
				registers.EIP = inst->address;
				continue; // skip EIP increment
			}
			break;
		}
		case ISA::Opcodes::XCHG:
		{
			const ISA::XCHG* inst = static_cast<const ISA::XCHG*>(base_inst);
			U32 tmp = registers.read(inst->destination);
			registers.write(inst->destination, registers.read(inst->source));
			registers.write(inst->source, tmp);
			break;
		}
		case ISA::Opcodes::STOP:
		{
			registers.EIP = instructions_count;
			std::cout << "Stop reached.\n";
			continue;
		}
		default:
			std::cout << "Unknown instruction: " << std::hex << (int) base_inst->opcode() << std::dec << "\n";
			return 1;
		}
		registers.EIP++;
	}
 
	std::cout << "\nFinal state reached in " << cycle << " cycles.\n";
	std::cout << "EAX: " << registers.read(ISA::Registers::EAX) << " (" << std::bitset<32>(registers.read(ISA::Registers::EAX)) << ")\n";
	std::cout << "EDX: " << registers.read(ISA::Registers::EDX) << " (" << std::bitset<32>(registers.read(ISA::Registers::EDX)) << ")\n";

	// cleaning
	for (int i = 0; i < instructions_count; i++) {
		if (instructions[i] != NULL) {
			delete instructions[i];
		}	
	}
	delete instructions;
}
