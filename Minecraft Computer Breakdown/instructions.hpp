#pragma once

#include <exception>

#include "data_types.h"


#define _OP_DEF(name, instruction_class) struct name : public instruction_class { using instruction_class::instruction_class; virtual U8 opcode() const { return Opcodes::name; } }


struct Inst
{
	U8 opcode;

	bit addressing_mode = 0;
	I32 displacement = 0;
	U32 immediate = 0;

	// types: None, Immediate, Memory, Register
	enum OperandType { None, I, M, R }
		op1_type = None, op2_type = None;
	bit op1_size = 0, op2_size = 0; // 0 -> 32 - 1 -> 8 or 16 if addressing_mode == 1
	U8 op1 = 0, op2 = 0;
};

void sizeFromRegister(U8 register_, bit& size, bit& addressing_mode)
{
	if (register_ < 8) {
		size = 0;
		addressing_mode = 0;
	}
	else if (register_ < 16) {
		size = 1;
		addressing_mode = 1;
	}
	else if (register_ < 24) {
		size = 1;
		addressing_mode = 0;
	}
	else if (register_ < 32) {
		size = 1;
		addressing_mode = 0;
	}
}


namespace ISA 
{
	namespace Registers
	{
		const U8 EAX = 0;
		const U8 ECX = 1;
		const U8 EDX = 2;
		const U8 EBX = 3;
		const U8 ESP = 4;
		const U8 EBP = 5;
		const U8 ESI = 6;
		const U8 EDI = 7;

		const U8 AX = 8;
		const U8 CX = 9;
		const U8 DX = 10;
		const U8 BX = 11;
		const U8 SP = 12;
		const U8 BP = 13;
		const U8 SI = 14;
		const U8 DI = 15;

		const U8 AH = 16;
		const U8 CH = 17;
		const U8 DH = 18;
		const U8 BH = 19;
		/*const U8 SPH = 20;
		const U8 BPH = 21;
		const U8 SIH = 22;
		const U8 DIH = 23;*/

		const U8 AL = 24;
		const U8 CL = 25;
		const U8 DL = 26;
		const U8 BL = 27;
		/*const U8 SPL = 28;
		const U8 BPL = 29;
		const U8 SIL = 30;
		const U8 DIL = 31;*/
	};

	namespace Opcodes
	{
		// Approximate opcodes. Usually the lowest possible for each mnemonic.
		const U8 ADD = 0x00;
		
		const U8 JO = 0x70;
		const U8 JNO = 0x71;
		
		const U8 NOP = 0x90;
		const U8 XCHG = 0x90;
		
		const U8 MOV = 0xA0;
		
		const U8 MUL = 0xF6;

		const U8 STOP = 0xFF; // temp
	};

	struct Instruction {
		virtual U8 opcode() const { return -1; }
	};
	
	struct MemoryInstruction : public Instruction
	{
		MemoryInstruction(U8 destination, U8 source) : destination(destination), source(source) {}

		U8 destination;
		U8 source;
	};
	
	struct ArithmeticInstruction : public Instruction
	{
		ArithmeticInstruction(U8 destination, U8 operand) : destination(destination), operand(operand) {}

		U8 destination;
		U8 operand;
	};

	struct JumpInstruction : public Instruction
	{
		JumpInstruction(U32 address) : address(address) {}

		U32 address;
	};
	
	const Inst* ADD(Inst::OperandType op1_type,
				    U8 op1, bit op1_size = 0,
			 	    Inst::OperandType op2_type = Inst::None,
			 	    U8 op2 = 0, bit op2_size = 0,
				    bit mode_16bit = 0,
					I32 displacement = 0,
					U32 immediate = 0)
	{
		if (op1_type == Inst::R) {
			sizeFromRegister(op1, op1_size, mode_16bit);
		}
		if (op2_type == Inst::R) {
			sizeFromRegister(op2, op2_size, mode_16bit);
		}
		
		return new Inst{
			ISA::Opcodes::ADD,
			mode_16bit,
			displacement,
			immediate,
			op1_type, op2_type,
			op1_size, op2_size,
			op1,	  op2
		};
	}
	
	

	// FIXME: incorrect opcodes
	//_OP_DEF(ADD, ArithmeticInstruction);

	_OP_DEF(JO, JumpInstruction); 
	_OP_DEF(JNO, JumpInstruction);
	
	_OP_DEF(XCHG, MemoryInstruction);
	_OP_DEF(MOV, MemoryInstruction);
	
	_OP_DEF(STOP, Instruction);

	namespace Memory
	{
		using ISA::MOV;
		using ISA::XCHG;
	};

	namespace Arithmetic
	{
		using ISA::ADD;
	};

	namespace Jumps
	{
		using ISA::JO;
		using ISA::JNO;
	};
}
