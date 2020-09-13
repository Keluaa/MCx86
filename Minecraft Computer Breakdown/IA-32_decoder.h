#pragma once

#include "instructions.hpp"

// TODO

class inst_decoder_exception : public std::exception
{
public:
	const char* msg;
	int pos;

	inst_decoder_exception(const char* msg, int pos)
		: msg(msg), pos(pos) {}
};


inline void sizeFromRegister(U8 register_, bit& size, bit& addressing_mode)
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

	const bit isLongOpcode() const { return (opcode & 0xFF00) == 0x0F; }

	const U8 getOpcode() const { return opcode & 0xFF; }
};

static Instruction_2 fromEncodedInstruction(const U8* bytes, int& pos)
{
	// see the reference used at section 2.1 - Instruction Format,
	// and also at the Appendixes A and B of the Volume 2
	// of the Intel 64 and IA-32 Architectures Software Developer's Manual
	// at https://software.intel.com/sites/default/files/managed/39/c5/325462-sdm-vol-1-2abcd-3abcd.pdf

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
	while (true)
	{
		switch (bytes[pos])
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
