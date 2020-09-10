 
#include <iostream>
#include <bitset>
 
#include "data_types.h"
#include "ALU.hpp"
#include "registers.h"
#include "RAM.h"
#include "ROM.h"
#include "instructions.hpp"

#ifdef _MSC_VER
// use a lambda to define an alternative for using descriptors in aggregate constructors
// see https://stackoverflow.com/a/49572324/8662187
#define with_new(T, param, ...) ([&]{ T ${param}; __VA_ARGS__; return new T($); }())
#else
// Should only be used when the compiler supports descriptors
#define $  // clear the $ sign used for the 'with_new' in the other case
#define with_new(T, param, ...) new T{param, __VA_ARGS__} // standard definition
#endif

void load_fibbonacci(Inst** instructions, const int instructions_count)
{
	if (instructions_count < 6) {
		std::cout << "Not enough space for fibbonacci program.\n";
		return;
	}

	int i = 0;
	instructions[i++] = with_new(Inst, ISA::Opcodes::MOV, $.op1_type = Inst::R, $.op1 = ISA::Registers::EAX, $.op2_type = Inst::I, $.op2 = 0);
	instructions[i++] = with_new(Inst, ISA::Opcodes::MOV, $.op1_type = Inst::R, $.op1 = ISA::Registers::EDX, $.op2_type = Inst::I, $.op2 = 1);
	instructions[i++] = with_new(Inst, ISA::Opcodes::ADD, $.op1_type = Inst::R, $.op1 = ISA::Registers::EAX, $.op2_type = Inst::R, $.op2 = ISA::Registers::EDX);
	instructions[i++] = with_new(Inst, ISA::Opcodes::XCHG, $.op1_type = Inst::R, $.op1 = ISA::Registers::EAX, $.op2_type = Inst::R, $.op2 = ISA::Registers::EDX);
	instructions[i++] = with_new(Inst, ISA::Opcodes::JNO, $.displacement = -2);
	instructions[i++] = with_new(Inst, ISA::Opcodes::STOP);

	/*
	instructions[i++] = new Inst{ISA::Opcodes::MOV, $.op1_type=Inst::R, .op1=ISA::Registers::EAX, .op2_type=Inst::I, .op2=0};
	instructions[i++] = new Inst{ISA::Opcodes::MOV, .op1_type=Inst::R, .op1=ISA::Registers::EDX, .op2_type=Inst::I, .op2=1};
	instructions[i++] = new Inst{ISA::Opcodes::ADD, .op1_type=Inst::R, .op1=ISA::Registers::EAX, .op2_type=Inst::R, .op2=ISA::Registers::EDX};
	instructions[i++] = new Inst{ISA::Opcodes::XCHG, .op1_type=Inst::R, .op1=ISA::Registers::EAX, .op2_type=Inst::R, .op2=ISA::Registers::EDX};
	instructions[i++] = new Inst{ISA::Opcodes::JNO, .displacement=-2};
	instructions[i++] = new Inst{ISA::Opcodes::STOP};
	*/
	
	/*
	instructions[i++] = new ISA::MOV(ISA::Registers::EAX, U8(0));
	instructions[i++] = new ISA::MOV(ISA::Registers::EDX, U8(1));
	instructions[i++] = ISA::ADD(Inst::R, ISA::Registers::EAX, IS, ISA::Registers::EDX);
	instructions[i++] = new ISA::XCHG(ISA::Registers::EAX, ISA::Registers::EDX);
	instructions[i++] = new ISA::JNO(2);
	instructions[i++] = new ISA::STOP();
	*/
}




void update_overflow_flag(Registers& registers, U32 op1, U32 op2, U32 result)
{
	/*
	overflow flag:
	1 2 R OF
	+ + + 0
	+ + - 1
	+ - / 0
	- + / 0
	- - - 0
	- - + 1

	=> (s(op1) & s(op2)) ^ s(R)
	*/
	registers.flag_write_OF((ALU::check_is_negative(op1) & ALU::check_is_negative(op2)) ^ ALU::check_is_negative(result));
}

void update_sign_flag(Registers& registers, U32 result)
{
	registers.flag_write_SF(ALU::check_is_negative(result));
}

void update_zero_flag(Registers& registers, U32 result)
{
	registers.flag_write_ZF(ALU::check_equal_zero(result));
}

void update_adjust_flag(Registers& registers, U32 op1, U32 op2, U8 register_)
{
	/*
	Adjust flag is set only if there were an carry from the first 4 bits of the AL register to the 4 other bits.
	It is 0 otherwise, including when the operation didn't used the AL register.
	This function should only be called with instructions modifing the AL register (or AX and EAX, but not AH).
	*/
	switch (register_)
	{
	case ISA::Registers::AL:
	case ISA::Registers::AX:
	case ISA::Registers::EAX:
		// Not the implementation used in the circuit, which is much simpler as this flag comes out from the adder
		// directly.
		registers.flag_write_AF((op1 & 0x0F) + (op2 & 0x0F) > 0x0F);
		break;

	default:
		registers.flag_write_AF(0);
		break;
	}
}

void update_parity_flag(Registers& registers, U32 result)
{
	/*
	Parity check is made only on the first byte
	*/
	registers.flag_write_PF(ALU::check_parity(U8(result)));
}

void update_carry_flag(Registers& registers, bit carry)
{
	registers.flag_write_CF(carry);
}

void update_status_flags(Registers& registers, U32 op1, U32 op2, U32 result, U8 register_, bit carry = 0)
{
	// updates all status flags
	update_overflow_flag(registers, op1, op2, result);
	update_sign_flag(registers, result);
	update_zero_flag(registers, result);
	update_adjust_flag(registers, op1, op2, register_);
	update_parity_flag(registers, result);
	update_carry_flag(registers, carry);
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

		bit op1_val_out_set = 0;
		U32 op1_val_out = 0;
		
		// perform the instruction
		
		switch (inst->opcode)
		{
		case ISA::Opcodes::ADD:
		{
			bit carry = 0;
			op1_val_out = ALU::add(op1_val, op2_val, carry);
			op1_val_out_set = 1;
			update_status_flags(registers, op1_val, op2_val, op1_val_out, (inst->op1_type == Inst::R ? inst->op1 : -1), carry);
		}
		case ISA::Opcodes::MOV:
		{
			op1_val_out = op2_val;
			op1_val_out_set = 1;
			break;
		}
		case ISA::Opcodes::JO:
		{
			if (registers.flag_read_OF()) {
				registers.EIP = op1_val;
				continue; // skip EIP increment
			}
			break;
		}
		case ISA::Opcodes::JNO:
		{
			if (!registers.flag_read_OF()) {
				registers.EIP = op1_val;
				continue; // skip EIP increment
			}
			break;
		}
		case ISA::Opcodes::XCHG:
		{
			// XCHG is restricted to exchange anything with the accumulator register.
			// here it is a general implementation for simplicity.


			U32 tmp = registers.read(inst->op);
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
		
		if (op1_val_out_set) {
			switch (inst->op1_type)
			{
			case Inst::R: // Register
				registers.write(inst->op1, op1_val_out);
				break;

			case Inst::M: // Memory
				ram.write(inst->op1, op1_val_out);
				break;

			case Inst::I: // Immediate
			case Inst::None:
				break;
			}
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
