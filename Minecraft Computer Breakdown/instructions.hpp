#pragma once

#include <exception>
#include <map>

#include "data_types.h"


#define _OP_DEF(name, instruction_class) struct name : public instruction_class { using instruction_class::instruction_class; virtual U8 opcode() const { return Opcodes::name; } }


struct Inst
{
	U8 opcode;

	bit two_bytes_opcode = 0;

	bit addressing_mode = 0;
	I32 displacement = 0;
	U32 immediate = 0;

	// types: None, Immediate, Memory, Register
	enum OperandType { None, I, M, R }
		op1_type = None, op2_type = None;
	
	// TODO: replace this by the operand size override prefix value
	bit op1_size = 0, op2_size = 0; // 0 -> 32 - 1 -> 8 or 16 if addressing_mode == 1, not used yet
	U8 op1 = 0, op2 = 0;
};

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

		const U8 AL = 20;
		const U8 CL = 21;
		const U8 DL = 22;
		const U8 BL = 23;
		
		const U8 EAX_EDX = 24; // used only for 64 bit assignements
	};

	namespace Opcodes
	{
		// Approximate opcodes. Usually the lowest possible for each mnemonic.
		const U8 ADD = 0x00;

		const U8 ADC = 0x10;
		
		const U8 AND = 0x20;
		
		const U8 DAA = 0x27;
		
		const U8 DAS = 0x2F;

		const U8 AAA = 0x37;
		const U8 CMP = 0x38;

		const U8 AAS = 0x3F;
		
		const U8 DEC = 0x48;
		
		const U8 BOUND = 0x62;
		const U8 ARPL = 0x63;
		
		const U8 IMUL = 0x69;
		const U8 JO = 0x70;
		const U8 JNO = 0x71;
		
		const U8 NOP = 0x90;
		const U8 XCHG = 0x90;
		
		const U8 CBW = 0x98;
		const U8 CWD = 0x99;

		const U8 CALL = 0xE8; // the only opcode implemented
		
		const U8 MOV = 0xA0;
		
		const U8 CMPS = 0xA6;
		
		const U8 ENTER = 0xC8;

		const U8 AAM = 0xD4;
		const U8 AAD = 0xD5;
		
		const U8 HLT = 0xF4;
		const U8 CMC = 0xF5;
		const U8 MUL = 0xF6;
		const U8 DIV = 0xF6;
		const U8 IDIV = 0xF6;
		
		const U8 CLC = 0xF8;
		
		const U8 CLI = 0xFA;
		
		const U8 CLD = 0xFC;
		
		const U8 STOP = 0xFF; // specific to this implementation, not present in the circuit

		// Two bytes opcodes, with the 0x0F prefix

		const U16 CLTS = 0x0F06;
		
		const U16 BT = 0x0FA3;

		const U16 BTS = 0x0FAB;

		const U16 BTR = 0x0FB3;
		
		const U16 BTC = 0x0FBB;
		const U16 BSF = 0x0FBC;
		const U16 BSR = 0x0FBD;
	};
}
