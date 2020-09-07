#pragma once

#include "data_types.h"


#define _OP_DEF(name, opcode, instruction_class) struct name : public instruction_class<opcode> { using instruction_class::instruction_class; }


namespace ISA 
{
	namespace Resigters
	{
		U8 EAX = 1;
		U8 ECX = 2;
		U8 EDX = 3;
	};

	struct BaseInstruction {};

	template<U8 opcode>
	struct Instruction : public BaseInstruction {};

	template<U8 opcode>
	struct MemoryInstruction : public Instruction<opcode>
	{
		MemoryInstruction(U8 source, U8 destination) : source(source), destination(destination) {}

		U8 source;
		U8 destination;
	};

	template<U8 opcode>
	struct ArithmeticInstruction : public Instruction<opcode>
	{
		ArithmeticInstruction(U8 destination, U8 operand) : destination(destination), operand(operand) {}

		U8 destination;
		U8 operand;
	};

	template<U8 opcode>
	struct JumpInstruction : public Instruction<opcode>
	{
		JumpInstruction(U32 address) : address(address) {}

		U32 address;
	};


	
	_OP_DEF(ADD, 0x00, ArithmeticInstruction);

	_OP_DEF(JO, 0x70, JumpInstruction); // FIXME: incorrect opcode
		
	_OP_DEF(XCHG, 0x90, MemoryInstruction);
	_OP_DEF(MOV, 0xA0, MemoryInstruction);

	_OP_DEF(NOP, 0x90, Instruction);
	
	
	
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