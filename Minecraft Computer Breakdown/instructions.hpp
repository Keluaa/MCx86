#pragma once

#include <exception>

#include "data_types.h"


#define _OP_DEF(name, instruction_class) struct name : public instruction_class { using instruction_class::instruction_class; virtual U8 opcode() const { return Opcodes::name; } }

class inst_decoder_exception : public std::exception 
{
public:
	const char* msg;
	int pos;
	
	inst_decoder_exception(const char* msg, int pos)
		: msg(msg), pos(pos) {}
};

struct Instruction_2
{
	U8 prefix_repeat;
	U8 prefix_segment;
	U8 prefix_operand_size;
	U8 prefix_address_size;
	U16 opcode;
	U8 modrm;
	U8 sib;
	U32 displacement;
	U32 immediate;
	
	static Instruction_2 fromEncodedInstruction(const U8* bytes, int& pos)
	{
		// see the reference used at section 17.2 - Instruction Format,
		// of the Intel manual for the 80386 processor architecture.
		
		U8 prefix_repeat = 0;
		U8 prefix_segment = 0;
		U8 prefix_operand_size = 0;
		U8 prefix_address_size = 0;
		U16 opcode = 0;
		U8 modrm = 0;
		U8 sib = 0;
		U32 displacement = 0;
		U32 immediate = 0;
		
		// handle possible prefixes
		while(true)
		{
			switch(bytes[pos])
			{
			// Lock & repeat prefixes
			case 0xF0: // LOCK
			case 0xF2: // REPNE / REPNZ
			case 0xF3: // REP or REPE / REPZ
				if (prefix_repeat != 0) {
					throw inst_decoder_exception("Prefix of same group already defined", pos);
				}
				prefix_repeat = bytes[pos];
				break;
					
			// Segment override prefixes
			case 0x2E: // CS
			case 0x36: // SS
			case 0x3E: // DS
			case 0x26: // ES
			case 0x64: // FS
			case 0x65: // GS
				if (prefix_segment != 0) {
					throw inst_decoder_exception("Prefix of same group already defined", pos);
				}
				prefix_segment = bytes[pos];
				break;
					
			case 0x66: // Operand size override prefix
				if (prefix_operand_size != 0) {
					throw inst_decoder_exception("Prefix of same group already defined", pos);
				}
				prefix_operand_size = bytes[pos];
				break;
						
			case 0x67: // Address size override prefix
				if (prefix_address_size != 0) {
					throw inst_decoder_exception("Prefix of same group already defined", pos);
				}
				prefix_address_size = bytes[pos];
				break;
						
			default:
				goto prefixes_parsed;
			}
			pos++; // increment only after parsing a valid prefix
		}
		prefixes_parsed:
		
		// Opcode
		if (bytes[pos] == 0x0F) {
			// 2 bytes opcode
			opcode |= 0xFF << 8;
			pos++;
		}
		opcode |= bytes[pos];
		
		// Mod r/m byte
		
		
		// SIB byte (32 bit addressing only)
		
		
		return Instruction_2{
			// TODO
		};
	}
	
	const bit isLongOpcode() const { return (opcode & 0xFF00) == 0x0F; }
	
	const U8 getOpcode() const { return opcode & 0xFF; }
};


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
		const U8 ADD = 0x00;
		const U8 JO = 0x70;
		const U8 JNO = 0x71;
		const U8 XCHG = 0x90;
		const U8 MOV = 0xA0;
		const U8 NOP = 0x90;

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

	// FIXME: incorrect opcodes
	_OP_DEF(ADD, ArithmeticInstruction);

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
