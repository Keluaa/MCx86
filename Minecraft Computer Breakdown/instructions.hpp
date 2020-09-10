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
	bit op1_size = 0, op2_size = 0; // 0 -> 32 - 1 -> 8 or 16 if addressing_mode == 1, not used yet
	U8 op1 = 0, op2 = 0;
};


void sizeFromRegister(U8 register_, bit& size, bit& addressing_mode)
{
	if (register_ < 8) {
		// EAX to EDI
		size = 0;
		addressing_mode = 0;
	}
	else if (register_ < 16) {
		// AX to DI
		size = 1;
		addressing_mode = 1;
	}
	else if (register_ < 32) {
		// AH to BL
		size = 1;
		addressing_mode = 0;
	}
}


namespace ISA 
{
	namespace Registers
	{
		// arbitrairy values for each register

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

		const U8 AL = 24;
		const U8 CL = 25;
		const U8 DL = 26;
		const U8 BL = 27;
	};

	namespace Opcodes
	{
		// Approximate opcodes. Usually the lowest possible for each mnemonic.
		const U8 ADD = 0x00;

		const U8 IMUL = 0x0F;
		
		const U8 JO = 0x70;
		const U8 JNO = 0x71;
		
		const U8 NOP = 0x90;
		const U8 XCHG = 0x90;
		
		const U8 MOV = 0xA0;
		
		const U8 MUL = 0xF6;


		const U8 STOP = 0xFF; // specific to this implementation, not present in the circuit
	};
}
