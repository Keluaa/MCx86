#pragma once

#include <map>
#include <string>

#include "../data_types.h"


namespace Opcodes
{
	constexpr U8 arithmetic = 0;
	constexpr U8 not_arithmetic = 1 << 7;
	constexpr U8 state_machine = 0b11 << 5;
	constexpr U8 jmp = 1 << 6;
	constexpr U8 str = 1 << 5;
	
	// Arithmetic only instructions
    // TODO : merge all flags set / clear instructions into one
	
	constexpr U8 AAA	 = 0 | arithmetic;
	constexpr U8 AAD     = 1 | arithmetic;
	constexpr U8 AAM     = 2 | arithmetic;
	constexpr U8 AAS     = 3 | arithmetic;
	constexpr U8 ADC     = 4 | arithmetic;
	constexpr U8 ADD     = 5 | arithmetic;
	constexpr U8 AND     = 6 | arithmetic;
	constexpr U8 ARPL    = 7 | arithmetic;
	constexpr U8 BOUND   = 8 | arithmetic;
	constexpr U8 BSF     = 9 | arithmetic;
	constexpr U8 BSR     = 10 | arithmetic;
	constexpr U8 BT      = 11 | arithmetic;
	constexpr U8 BTC     = 12 | arithmetic;
	constexpr U8 BTR     = 13 | arithmetic;
	constexpr U8 BTS     = 14 | arithmetic;
	constexpr U8 CBW     = 15 | arithmetic;
	constexpr U8 CLC     = 16 | arithmetic;
	constexpr U8 CLD     = 17 | arithmetic;
	constexpr U8 CLI     = 18 | arithmetic;
	constexpr U8 CLTS    = 19 | arithmetic;
	constexpr U8 CMC     = 20 | arithmetic;
	constexpr U8 CMP     = 21 | arithmetic;
	constexpr U8 CWD     = 22 | arithmetic;
	constexpr U8 DAA     = 23 | arithmetic;
	constexpr U8 DAS     = 24 | arithmetic;
	constexpr U8 DEC     = 25 | arithmetic;
	constexpr U8 DIV     = 26 | arithmetic;
	constexpr U8 IDIV    = 27 | arithmetic;
	constexpr U8 IMUL    = 28 | arithmetic;
	constexpr U8 INC     = 29 | arithmetic;
	constexpr U8 LAHF    = 30 | arithmetic;
	constexpr U8 LEA     = 31 | arithmetic;
	constexpr U8 MOV     = 32 | arithmetic;
	constexpr U8 MOVSX   = 33 | arithmetic;
	constexpr U8 MOVZX   = 34 | arithmetic;
	constexpr U8 MUL     = 35 | arithmetic;
	constexpr U8 NEG     = 36 | arithmetic;
	constexpr U8 NOP     = 37 | arithmetic;
	constexpr U8 NOT     = 38 | arithmetic;
	constexpr U8 OR      = 39 | arithmetic;
	constexpr U8 ROT     = 40 | arithmetic; // heavily modified to fit in one instruction
	constexpr U8 SAHF    = 41 | arithmetic;
    constexpr U8 SHFT    = 42 | arithmetic; // heavily modified to fit in one instruction
	constexpr U8 SBB     = 43 | arithmetic;
	constexpr U8 SETcc   = 44 | arithmetic;
	constexpr U8 SHD     = 45 | arithmetic; // merged SHLD and SHRD
	constexpr U8 STC     = 46 | arithmetic;
	constexpr U8 STD     = 47 | arithmetic;
	constexpr U8 STI     = 48 | arithmetic;
	constexpr U8 SUB     = 49 | arithmetic;
	constexpr U8 TEST    = 50 | arithmetic;
	constexpr U8 XCHG    = 51 | arithmetic;
	constexpr U8 XLAT    = 52 | arithmetic;
	constexpr U8 XOR     = 53 | arithmetic;
	
	// Non-arithmetic instructions

	constexpr U8 HLT     = 0 | not_arithmetic;
	constexpr U8 IN      = 1 | not_arithmetic;
	constexpr U8 LAR     = 2 | not_arithmetic;
	constexpr U8 LGDT    = 3 | not_arithmetic;
	constexpr U8 LGS     = 4 | not_arithmetic;
	constexpr U8 LLDT    = 5 | not_arithmetic;
	constexpr U8 LMSW    = 6 | not_arithmetic;
	constexpr U8 LOCK    = 7 | not_arithmetic;
	constexpr U8 LSL     = 8 | not_arithmetic;
	constexpr U8 LTR     = 9 | not_arithmetic;
	constexpr U8 OUT     = 10 | not_arithmetic;
	constexpr U8 POP     = 11 | not_arithmetic;
	constexpr U8 POPF    = 12 | not_arithmetic;
	constexpr U8 PUSH    = 13 | not_arithmetic;
	constexpr U8 PUSHF   = 14 | not_arithmetic;
	constexpr U8 SGDT    = 15 | not_arithmetic;
	constexpr U8 SLDT    = 16 | not_arithmetic;
	constexpr U8 SMSW    = 17 | not_arithmetic;
	constexpr U8 STR     = 18 | not_arithmetic;
	constexpr U8 VERR    = 19 | not_arithmetic;
	constexpr U8 WAIT    = 20 | not_arithmetic;

	// Non-arithmetic instructions on strings
	
	constexpr U8 CMPS    = 0 | not_arithmetic | str; // TODO : implement
	constexpr U8 INS     = 1 | not_arithmetic | str; // TODO : implement
	constexpr U8 LODS    = 2 | not_arithmetic | str; // TODO : implement
	constexpr U8 MOVS    = 3 | not_arithmetic | str; // TODO : implement
	constexpr U8 OUTS    = 4 | not_arithmetic | str; // TODO : implement
	constexpr U8 SCAS    = 5 | not_arithmetic | str; // TODO : implement
	constexpr U8 STOS    = 6 | not_arithmetic | str; // TODO : implement
	
	// Non-arithmetic instructions with jumps
	
	constexpr U8 CALL    = 0 | not_arithmetic | jmp;
	constexpr U8 INT     = 1 | not_arithmetic | jmp; // TODO : implement
	constexpr U8 IRET    = 2 | not_arithmetic | jmp;
	constexpr U8 Jcc     = 3 | not_arithmetic | jmp;
	constexpr U8 JMP     = 4 | not_arithmetic | jmp;
	constexpr U8 LOOP    = 5 | not_arithmetic | jmp;
	constexpr U8 REP     = 6 | not_arithmetic | jmp; // TODO : implement
	constexpr U8 RET     = 7 | not_arithmetic | jmp;

	// State Machine instructions

	constexpr U8 ENTER   = 0 | not_arithmetic | state_machine;
	constexpr U8 LEAVE   = 1 | not_arithmetic | state_machine;
	constexpr U8 POPA    = 2 | not_arithmetic | state_machine;
	constexpr U8 PUSHA   = 3 | not_arithmetic | state_machine;
	
	// Custom instructions
	
	// TODO : check usefulness
	constexpr U8 IMULX   = 54 | arithmetic; // used after IMUL or MUL on 32 bit operands, to extend the result to 64 bit
	constexpr U8 MULX    = 55 | arithmetic; // used to perform 64 bit multiplication
	
	//  Mnemonics
	extern std::map<U8, std::string> mnemonics;
}
