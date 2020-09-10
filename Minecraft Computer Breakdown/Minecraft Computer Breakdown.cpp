 
#include <iostream>
#include <bitset>
 
#include "data_types.h"
#include "ALU.h"
#include "registers.h"
#include "RAM.h"
#include "ROM.h"
#include "instructions.hpp"

void load_fibbonacci(Inst** instructions, const int instructions_count)
{
	if (instructions_count < 6) {
		std::cout << "Not enough space for fibbonacci program.\n";
		return;
	}

	int i = 0;
	instructions[i++] = new Inst{ISA::Opcodes::MOV, .op1_type=Inst::R, .op1=ISA::Registers::EAX, .op2_type=Inst::I, .op2=0};
	instructions[i++] = new Inst{ISA::Opcodes::MOV, .op1_type=Inst::R, .op1=ISA::Registers::EDX, .op2_type=Inst::I, .op2=1};
	instructions[i++] = new Inst{ISA::Opcodes::ADD, .op1_type=Inst::R, .op1=ISA::Registers::EAX, .op2_type=Inst::R, .op2=ISA::Registers::EDX};
	instructions[i++] = new Inst{ISA::Opcodes::XCHG, .op1_type=Inst::R, .op1=ISA::Registers::EAX, .op2_type=Inst::R, .op2=ISA::Registers::EDX};
	instructions[i++] = new Inst{ISA::Opcodes::JNO, .displacement=-2};
	instructions[i++] = new Inst{ISA::Opcodes::STOP};
	
	/*
	instructions[i++] = new ISA::MOV(ISA::Registers::EAX, U8(0));
	instructions[i++] = new ISA::MOV(ISA::Registers::EDX, U8(1));
	instructions[i++] = ISA::ADD(Inst::R, ISA::Registers::EAX, IS, ISA::Registers::EDX);
	instructions[i++] = new ISA::XCHG(ISA::Registers::EAX, ISA::Registers::EDX);
	instructions[i++] = new ISA::JNO(2);
	instructions[i++] = new ISA::STOP();
	*/
}

int main()
{
	Registers registers;
	RAM<256> ram;
 
	const int instructions_count = 6;

	Inst** instructions = new Inst*[instructions_count];
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
 
		const Inst* inst = instructions[registers.EIP];
		
		// load values fron registers / memory / instruction
		
		U32 op1_val = 0;
		switch (inst->op1_type)
		{
		case Inst::I: // Immediate
			op1_val = inst->op1;
			break;
			
		case Inst::R: // Register
			op1_val = registers.read(inst->op1);
			break;
			
		case Inst::M: // Memory
			op1_val = ram.read(inst->op1);
			break;
			
		case Inst::None:
			break;
		}
		
		U32 op2_val = 0;
		switch (inst->op2_type)
		{
		case Inst::I: // Immediate
			op2_val = inst->op2;
			break;
			
		case Inst::R: // Register
			op2_val = registers.read(inst->op2);
			break;
			
		case Inst::M: // Memory
			op2_val = ram.read(inst->op2);
			break;
			
		case Inst::None:
			break;
		}
		
		// perform the instruction
		
		switch (inst->opcode)
		{
		case ISA::Opcodes::ADD:
		{
			/*
			TODO: put flag updates in dedicated functions
			overflow flag:
			1 2 R OF
			+ + + 0
			+ + - 1
			+ - / 0
			- + / 0
			- - - 0
			- - + 1
			
			=> (s(1) & s(2)) ^ s(R)
			*/
			bit carry = 0;
			bit overflow_possible = ALU::check_is_negative(op1_val) & ALU::check_is_negative(op2_val);
			op1_val = ALU::add(op1_val, op2_val, carry);
			registers.flag_write_CF(carry);
			registers.flag_write_OF(ALU::check_is_negative(op1_val) ^ overflow_possible);
			registers.flag_write_ZF(ALU::check_equal_zero(op1_val));
			registers.flag_write_SF(ALU::check_is_negative(op1_val));
			registers.flag_write_PF(ALU::check_parity(op1_val));
			registers.flag_write_AF(
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
		
		// write new values back to registers / memory
		
		switch (inst->op1_type)
		{
		case Inst::R: // Register
			registers.write(inst->op1, op1_val);
			break;
			
		case Inst::M: // Memory
			ram.write(inst->op1, op1_val);
			break;
		
		case Inst::I: // Immediate
		case Inst::None:
			break;
		}
		
		switch (inst->op2_type)
		{
		case Inst::R: // Register
			registers.write(inst->op2, op2_val);
			break;
			
		case Inst::M: // Memory
			ram.write(inst->op2, op2_val);
			break;
		
		case Inst::I: // Immediate
		case Inst::None:
			break;
		}
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
	delete[] instructions;
}
