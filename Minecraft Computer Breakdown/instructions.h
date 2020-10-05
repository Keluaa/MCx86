#pragma once

#include "data_types.h"


enum OpSize : U8
{
	// the most used sizes have the fewest one bits
	DW = 0b00,
	W = 0b01,
	B = 0b10,
	UNKNOWN = 0b11
};


enum OpType : U8 
{
	// the most used operand types have the fewest 1 bits
	NONE = 0b000,
	REG = 0b001,
	MEM = 0b010,
	IMM = 0b100,
	M_M = 0b011, // m16&16 or m32&32 operand
};


struct InstData
{
	U32 op1, op2, op3;
	
	U32 address;
	U32 imm;
	
	OpSize op1_size:2;
	OpSize op2_size:2;
	OpSize op3_size:2;
	U8:2; // padding
};


struct Inst
{
	/*
	Instruction encoding format:
		- 8 bits: opcode
		- 1 bit : address size override (0->32, 1->16), final size depends on external flags
		- 1 bit : operand size override (0->32, 1->16), final size depends on external flags
		- 1 bit : address byte size override
		- 1 bit : operand byte size override
		- 1 bit : access flags write them after the instruction execution
		- 3 bits: operand type of the first operand
		- 3 bits: operand type of the second operand
		- 3 bits: register of the first operand
		- 3 bits: register of the second operand
		- 1 bit : read the first operand source
		- 1 bit : read the second operand source
		- 1 bit : write the first return value to the first operand source
		- 1 bit : write the second return value to the second operand source
		- 1 bit : write the first return value to a specific register
		- 1 bit : scale this specific register
		- 3 bits: which register
		- 32 bits: address value (or offset ?)
		- 32 bits: immediate value			
	*/
	
	U8 opcode;

	// TODO : maybe combine 16 and 8 bits size overrides to a two flags only
	bit address_size_override:1;
	bit operand_size_override:1;
	bit address_byte_size_override:1;
	bit operand_byte_size_override:1;

	bit get_flags:1;
	
	OpType op1_type:3;
	OpType op2_type:3;

	U8 op1_register:3;
	U8 op2_register:3;
	
	bit read_op1:1;
	bit read_op2:1;

	bit write_ret1_to_op1:1;
	bit write_ret2_to_op2:1;

	bit write_ret1_to_register:1;
	bit scale_output_override:1;
	U8 register_out:3;
	
	U8:0; // alignment
	
	// both of those values can be used as general purpose values in spacial instrucions (bound, call...)
	U32 address_value; 
	U32 immediate_value;
	
	Inst(U8 opcode, U32 flags, U32 address_value = 0, U32 immediate_value = 0);
		
	InstData getInstData() const;
};


namespace Opcodes
{
	constexpr U8 arithmethic = 0;
	constexpr U8 not_arithmethic = 1 << 7;
	constexpr U8 jmp = 1 << 6;
	constexpr U8 str = 1 << 5;
	
	// Arithmetic only instructions
	
	constexpr U8 AAA	 =  0 | arithmethic;
	constexpr U8 AAD     =  1 | arithmethic;
	constexpr U8 AAM     =  2 | arithmethic;
	constexpr U8 AAS     =  3 | arithmethic;
	constexpr U8 ADC     =  4 | arithmethic;
	constexpr U8 ADD     =  5 | arithmethic;
	constexpr U8 AND     =  6 | arithmethic;
	constexpr U8 ARPL    =  7 | arithmethic;
	constexpr U8 BOUND   =  8 | arithmethic;
	constexpr U8 BSF     =  9 | arithmethic;
	constexpr U8 BSR     = 10 | arithmethic;
	constexpr U8 BT      = 11 | arithmethic;
	constexpr U8 BTC     = 12 | arithmethic;
	constexpr U8 BTR     = 13 | arithmethic;
	constexpr U8 BTS     = 14 | arithmethic;
	constexpr U8 CBW     = 15 | arithmethic;
	constexpr U8 CLC     = 16 | arithmethic;
	constexpr U8 CLD     = 17 | arithmethic;
	constexpr U8 CLI     = 18 | arithmethic;
	constexpr U8 CLTS    = 19 | arithmethic;
	constexpr U8 CMC     = 20 | arithmethic;
	constexpr U8 CMP     = 21 | arithmethic;
	constexpr U8 CWD     = 22 | arithmethic;
	constexpr U8 DAA     = 23 | arithmethic;
	constexpr U8 DAS     = 24 | arithmethic;
	constexpr U8 DEC     = 25 | arithmethic;
	constexpr U8 DIV     = 26 | arithmethic;
	constexpr U8 IDIV    = 27 | arithmethic;
	constexpr U8 IMUL    = 28 | arithmethic;
	constexpr U8 INC     = 29 | arithmethic;
	constexpr U8 LEA     = 30 | arithmethic;
	constexpr U8 MOV     = 31 | arithmethic;
	constexpr U8 MOVSX   = 32 | arithmethic;
	constexpr U8 MOVZX   = 33 | arithmethic;
	constexpr U8 MUL     = 34 | arithmethic;
	constexpr U8 NEG     = 35 | arithmethic;
	constexpr U8 NOP     = 36 | arithmethic;
	constexpr U8 NOT     = 37 | arithmethic;
	constexpr U8 OR      = 38 | arithmethic;
	constexpr U8 RCL     = 39 | arithmethic;
	constexpr U8 SAL     = 40 | arithmethic;
	constexpr U8 SBB     = 41 | arithmethic;
	constexpr U8 SETcc   = 42 | arithmethic;
	constexpr U8 SHRD    = 43 | arithmethic;
	constexpr U8 SLDT    = 44 | arithmethic;
	constexpr U8 STC     = 45 | arithmethic;
	constexpr U8 STD     = 46 | arithmethic;
	constexpr U8 STI     = 47 | arithmethic;
	constexpr U8 SUB     = 48 | arithmethic;
	constexpr U8 TEST    = 49 | arithmethic;
	constexpr U8 XCHG    = 50 | arithmethic;
	constexpr U8 XLAT    = 51 | arithmethic;
	constexpr U8 XOR     = 52 | arithmethic;
	
	// Non arithmetic instructions
	
	constexpr U8 ENTER   =  0 | not_arithmethic;
	constexpr U8 HLT     =  1 | not_arithmethic;
	constexpr U8 IN      =  2 | not_arithmethic;
	constexpr U8 LAHF    =  3 | not_arithmethic;
	constexpr U8 LAR     =  4 | not_arithmethic;
	constexpr U8 LGDT    =  5 | not_arithmethic;
	constexpr U8 LGS     =  6 | not_arithmethic;
	constexpr U8 LLDT    =  7 | not_arithmethic;
	constexpr U8 LMSW    =  8 | not_arithmethic;
	constexpr U8 LOCK    =  9 | not_arithmethic;
	constexpr U8 LSL     = 10 | not_arithmethic;
	constexpr U8 LTR     = 11 | not_arithmethic;
	constexpr U8 OUT     = 12 | not_arithmethic;
	constexpr U8 POP     = 13 | not_arithmethic;
	constexpr U8 POPA    = 14 | not_arithmethic;
	constexpr U8 POPF    = 15 | not_arithmethic;
	constexpr U8 PUSH    = 16 | not_arithmethic;
	constexpr U8 PUSHA   = 17 | not_arithmethic;
	constexpr U8 PUSHF   = 18 | not_arithmethic;
	constexpr U8 SAHF    = 19 | not_arithmethic;
	constexpr U8 SGDT    = 20 | not_arithmethic;
	constexpr U8 SMSW    = 21 | not_arithmethic;
	constexpr U8 STR     = 22 | not_arithmethic;
	constexpr U8 VERR    = 23 | not_arithmethic;
	constexpr U8 WAIT    = 24 | not_arithmethic;

	// Non arithmetic instructions on strings
	
	constexpr U8 CMPS    =  0 | not_arithmethic | str;
	constexpr U8 INS     =  1 | not_arithmethic | str;
	constexpr U8 LODS    =  2 | not_arithmethic | str;
	constexpr U8 MOVS    =  3 | not_arithmethic | str;
	constexpr U8 OUTS    =  4 | not_arithmethic | str;
	constexpr U8 SCAS    =  5 | not_arithmethic | str;
	constexpr U8 STOS    =  6 | not_arithmethic | str;
	
	// Non arithmetic instructions with jumps
	
	constexpr U8 CALL    =  0 | not_arithmethic | jmp;
	constexpr U8 INT     =  1 | not_arithmethic | jmp;
	constexpr U8 IRET    =  2 | not_arithmethic | jmp;
	constexpr U8 Jcc     =  3 | not_arithmethic | jmp;
	constexpr U8 JMP     =  4 | not_arithmethic | jmp;
	constexpr U8 LEAVE   =  5 | not_arithmethic | jmp;
	constexpr U8 LOOP    =  6 | not_arithmethic | jmp;
	constexpr U8 REP     =  7 | not_arithmethic | jmp | str;
	constexpr U8 RET     =  8 | not_arithmethic | jmp;
};
