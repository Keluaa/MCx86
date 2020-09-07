#pragma once

#include "data_types.h"


#define _OP_DEF(name, code, instruction_class) struct name : public instruction_class { const U8 opcode = code; using instruction_class::instruction_class; }


namespace ISA 
{
	namespace Resigters
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
		const U8 SPH = 20;
		const U8 BPH = 21;
		const U8 SIH = 22;
		const U8 DIH = 23;

		const U8 AL = 24;
		const U8 CL = 25;
		const U8 DL = 26;
		const U8 BL = 27;
		const U8 SPL = 28;
		const U8 BPL = 29;
		const U8 SIL = 30;
		const U8 DIL = 31;
	};

	namespace Opcodes
	{
		const U8 ADD = 0x00;
		const U8 JO = 0x70;
		const U8 XCHG = 0x90;
		const U8 MOV = 0xA0;
		const U8 NOP = 0x90;
	};

	struct Instruction {
		const U8 opcode = -1;
	};

	struct SomeInstruction : public Instruction 
	{
		SomeInstruction(U8 some_parameter) : some_parameter(some_parameter) {}

		U8 some_parameter;
	};

	struct MemoryInstruction : public Instruction
	{
		MemoryInstruction(U8 source, U8 destination) : source(source), destination(destination) {}

		U8 source;
		U8 destination;
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


	_OP_DEF(ADD, 0x00, ArithmeticInstruction);

	_OP_DEF(JO, 0x70, JumpInstruction); // FIXME: incorrect opcode
		
	_OP_DEF(XCHG, 0x90, MemoryInstruction);
	_OP_DEF(MOV, 0xA0, MemoryInstruction);
	

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
	};
}