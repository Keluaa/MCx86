#pragma once

#include <map>

#include "data_types.h"
#include "instructions.hpp"


struct SizeOverrides
{
	bit address_size_override:1;
	bit operand_size_override:1;
	bit address_byte_size_override:1;
	bit operand_byte_size_override:1;
};


enum OpSize : U8 {
	DW = 0b00,
	W = 0b01,
	B = 0b10
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

struct Inst_2
{
	/*
	TODO: update
	
	Wait, is this how you are supposed to make a decoder ???
	New Instruction format propositions:
	 - ? 8-16 bits: opcode
	 - 1 bit: address size override (0->32, 1->16), final size depends on external flags
	 - 1 bit: operand size override (0->32, 1->16), final size depends on external flags
	 - 1 bit: address byte size override
	 - 1 bit: operand byte size override
	 - 1 bit: write result to destination operand
	 - 1 bit: destination override: write the result to a specific register
	 - 5 bits : which register 
	 	-> 0-7: EAX - EDI
		-> 8-15: AX - DI
		-> 16-23: AL - BL
		-> 24: EAX:EDX
		-> 25: AX:DX
		-> 26-31: ?
	 - 1 bit: allow flags update ?
	
	 - 1 bit: presence of register as first operand
	 - 1 bit: presence of address (or offset ?) as first/second operand
	 - 1 bit: presence of register as second operand
  	 - 1 bit: presence of immediate after other operands
	 
	 - 3 bits: which first register (0-7, size controlled by override bits)
	 - 3 bits: which second register (0-7, size controlled by override bits)
	
	 - 32 bits: address value (or offset ?)
	 - 32 bits: immediate value
	
	
	New opcode encoding proposal:
	 - caregory bits:
		- Normal Arithmetic: update all of the flags depending on the unique result
		- Jumps/Calls/Rets: no flags, sets EIP manually
		- Strings ops: m8, m8 operands, can be repeated through the whole string
		- ...
		- other: no constraints
	
	New New opcode encoding proposal:
		- 1 bit: trivial operation: most of arithmetic operations, etc...
				-> can be executed without complex wiring
		- trivial op:
			-> 2 bits for ALU ops
			
	*/
	
	U8 opcode;

	bit address_size_override:1;
	bit operand_size_override:1;
	bit address_byte_size_override:1; // TODO : maybe combine byte size overrides to a single one
	bit operand_byte_size_override:1;
	
	bit write_to_dest:1;
	bit register_out_override:1;
	U8 register_out:5;
	
	/*
	Je veux:
	 - rien faire
	 - ecrire dans le dest, en fonction des params de l'inst
	 - ecrire dans un registre specifique
	 - ecrire dans un registre specifique, en fonction des params de l'inst
	 - ecrire dans deux registres, en fonction des params de l'inst
	 - ecrire dans deux registres specifiques, en fonction des params de l'inst
	
	Proposition:
	 - 1 bit: scale both outputs with inst size params (including overrides)
	 - 1 bit: write result 1 to op1 source
	 - 1 bit: write result 2 to op2 source
	 - 1 bit: both outputs overrides
	 - 5 bit: register out 1
	 - 5 bit: register out 2
	*/
	
	bit get_flags:1;
	
	enum OpType : U8 { // the most used operand types have the fewest 1 bits
		NONE = 0b000, 
		REG = 0b001,
		MEM = 0b010,
		IMM = 0b100,
		M_M = 0b011, // m16&16 or m32&32 operand
	};
	
	U8 op1_type:3;
	U8 op2_type:3;
	
	bit read_op1:1;
	bit read_op2:1;
	
	U8 op1_register:3;
	U8 op2_register:3;
	
	U8:2; // alignment
	
	// both of those values can be used as general purpose values in spacial instrucions (bound, call...)
	U32 address_value; 
	U32 immediate_value;
	
	Inst_2(U16 opcode,  U32 flags, U32 address_value = 0, U32 immediate_value = 0)
#if 1
	;
#else
		: opcode(opcode), address_value(address_value), immediate_value(immediate_value)
	{
		address_size_override = (flags >> 23) & 1;
		operand_size_override = (flags >> 22) & 1;
		address_byte_size_override = (flags >> 21) & 1;
		operand_byte_size_override = (flags >> 20) & 1;
		write_to_dest = (flags >> 19) & 1;
		register_out_override = (flags >> 18) & 1;
		register_out = (flags >> 13) & 0b11111;
		flags_update = (flags >> 12) & 1;
		/*is_op1_register = (flags >> 11) & 1;
		is_next_op_address = (flags >> 10) & 1;
		is_next_op_register = (flags >> 9) & 1;
		is_next_op_immediate = (flags >> 8) & 1;*/
		op1_type = (flags >> 10) & 0b11;
		op2_type = (flags >> 8) & 0b11;
		read_op1 = (flags >> 7) & 1;
		read_op2 = (flags >> 6) & 1;
		op1_register = (flags >> 3) & 0b111;
		op2_register = (flags >> 0) & 0b111;
	}
#endif

	SizeOverrides getSizeOverrides() const 
	{
		return SizeOverrides{
			address_size_override,
			operand_size_override,
			address_byte_size_override,
			operand_byte_size_override
		};
	}
	
	InstData getInstData() const
	{
		return InstData{
			0, 0,
			address_value,
			immediate_value
		};
	}
};

#if 0
#define size16 (0b11 << 22) // sets the operand and address sizes to two bytes
#define size8 (0b11 << 20) // sets the operand and address sizes to one byte
#define write_dest (0b1 << 19) // sets write to dest
#define out_override(reg) (0b1 << 18) | (reg << 13) // sets the register override
#define flags_update (0b1 << 12) // some flags will be updated
/*
#define reg_reg (0b1010 << 8) // operands types
#define mem_reg (0b0110 << 8)
#define reg_mem (0b1100 << 8)
#define mem_imm (0b0101 << 8)
#define reg_imm (0b1001 << 8)
#define reg_r_m (0b1000 << 8) // reg, r/m -> should be specified
#define r_m_reg (0b0010 << 8) // r/m, reg -> should be specified
#define r_m_imm (0b0001 << 8) // r/m, imm -> should be specified
#define reg1(reg) (reg << 5) // register operands
#define reg2(reg) (reg << 2)
*/
// operands specification
#define op1r(read) (read << 7)
#define op2r(read) (read << 6)
#define op1t(type, read) ((type << 10) | op1r(read))
#define op2t(type, read) ((type << 8) | op2r(read))
#define reg1(reg) (reg << 3) // register operands
#define reg2(reg) (reg << 0)

// operand types
#define reg 0b01
#define mem 0b10
#define imm 0b11

// used to specify if an operand has its value read
#define read 1
#define no_read 0

// sizes depends on the size prefixes or if the instructions works with one byte
#define reg_A 0  // EAX, AX, AL
#define reg_C 1  // ECX, CX, CL
#define reg_D 2  // EDX, DX, DL
#define reg_B 3  // EBX, BX, BL
#define reg_SP 4 // ESP, SP, AH
#define reg_BP 5 // EBP, BP, CH
#define reg_SI 6 // ESI, SI, DH
#define reg_DI 7 // EDI, DI, BH

// specifies an output register, indexed by the global index
// the register_out_override flag is automatically set
#define out_reg(reg) ((reg << 13) | (1 << 18))

// handle opcode extensions (stored in the mod r/m byte of the instruction)
// by adding the 3 bits to the last 4 bits of the opcode.
// not the official solution, but it should work
#define OPEXT(d) + (d << 12)

#endif


namespace Opcodes_2
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


namespace ISA
{
	extern const std::map<U16, Inst_2> InstTable
#if 1
	;
#else
	{
/* AAA */	{ 0x37, Inst_2(ISA::Opcodes::AAA, write_dest | flags_update | size16 | reg1(reg_A) | op1t(reg, read)) },
/* AAD */ { 0xD50A, Inst_2(ISA::Opcodes::AAD, write_dest | flags_update | size16 | reg1(reg_A) | op1t(reg, read)) },
/* AAM */ { 0xD40A, Inst_2(ISA::Opcodes::AAM, write_dest | flags_update | size16 | reg1(reg_A) | op1t(reg, read)) },
/* AAS */	{ 0x3F, Inst_2(ISA::Opcodes::AAM, write_dest | flags_update | size16 | reg1(reg_A) | op1t(reg, read)) },

/* ADC */	{ 0x10, Inst_2(ISA::Opcodes::ADC, write_dest | flags_update | op1r(read) | op2t(reg, read) | size8) },
			{ 0x11, Inst_2(ISA::Opcodes::ADC, write_dest | flags_update | op1r(read) | op2t(reg, read)) },
			{ 0x12, Inst_2(ISA::Opcodes::ADC, write_dest | flags_update | op1t(reg, read) | op2r(read) | size8) },
			{ 0x13, Inst_2(ISA::Opcodes::ADC, write_dest | flags_update | op1t(reg, read) | op2r(read)) },
			{ 0x14, Inst_2(ISA::Opcodes::ADC, write_dest | flags_update | op1t(reg, read) | op2t(imm, read) | reg1(reg_A) | size8) },
			{ 0x15, Inst_2(ISA::Opcodes::ADC, write_dest | flags_update | op1t(reg, read) | op2t(imm, read) | reg1(reg_A)) },
			{ 0x80 OPEXT(2), Inst_2(ISA::Opcodes::ADC, write_dest | flags_update | op1r(read) | op2t(imm, read) | size8) },
			{ 0x81 OPEXT(2), Inst_2(ISA::Opcodes::ADC, write_dest | flags_update | op1r(read) | op2t(imm, read)) },
			{ 0x83 OPEXT(2), Inst_2(ISA::Opcodes::ADC, write_dest | flags_update | op1r(read) | op2t(imm, read)) },
		
/* ADD */	{ 0x00, Inst_2(ISA::Opcodes::ADD, write_dest | flags_update | op1r(read) | op2t(reg, read) | size8) },
			{ 0x01, Inst_2(ISA::Opcodes::ADD, write_dest | flags_update | op1r(read) | op2t(reg, read)) },
			{ 0x02, Inst_2(ISA::Opcodes::ADD, write_dest | flags_update | op1t(reg, read) | op2r(read) | size8) },
			{ 0x03, Inst_2(ISA::Opcodes::ADD, write_dest | flags_update | op1t(reg, read) | op2r(read)) },
			{ 0x04, Inst_2(ISA::Opcodes::ADD, write_dest | flags_update | reg1(reg_A) | op2t(imm, read) | size8) },
			{ 0x05, Inst_2(ISA::Opcodes::ADD, write_dest | flags_update | reg1(reg_A) | op2t(imm, read)) },
			{ 0x80 OPEXT(0), Inst_2(ISA::Opcodes::ADD, write_dest | flags_update | op1r(read) | op2t(imm, read) | size8) },
			{ 0x81 OPEXT(0), Inst_2(ISA::Opcodes::ADD, write_dest | flags_update | op1r(read) | op2t(imm, read)) },
			{ 0x83 OPEXT(0), Inst_2(ISA::Opcodes::ADD, write_dest | flags_update | op1r(read) | op2t(imm, read)) },
			
/* AND */	{ 0x20, Inst_2(ISA::Opcodes::AND, write_dest | flags_update | op1r(read) | op2t(reg, read) | size8) },
			{ 0x21, Inst_2(ISA::Opcodes::AND, write_dest | flags_update | op1r(read) | op2t(reg, read)) },
			{ 0x22, Inst_2(ISA::Opcodes::AND, write_dest | flags_update | op1t(reg, read) | op2r(read) | size8) },
			{ 0x23, Inst_2(ISA::Opcodes::AND, write_dest | flags_update | op1t(reg, read) | op2r(read)) },
			{ 0x24, Inst_2(ISA::Opcodes::AND, write_dest | flags_update | op1t(reg, read) | op2t(imm, read) | reg1(reg_A) | size8) },
			{ 0x25, Inst_2(ISA::Opcodes::AND, write_dest | flags_update | op1t(reg, read) | op2t(imm, read) | reg1(reg_A)) },
			{ 0x80 OPEXT(4), Inst_2(ISA::Opcodes::AND, write_dest | flags_update | op1r(read) | op2t(imm, read) | size8) },
			{ 0x81 OPEXT(4), Inst_2(ISA::Opcodes::AND, write_dest | flags_update | op1r(read) | op2t(imm, read)) },
			{ 0x83 OPEXT(4), Inst_2(ISA::Opcodes::AND, write_dest | flags_update | op1r(read) | op2t(imm, read)) },
			
/* ARPL */	{ 0x63, Inst_2(ISA::Opcodes::ARPL, write_dest | flags_update | op1r(read) | op2t(reg, read) | size16) },
/* BOUND */	{ 0x62, Inst_2(ISA::Opcodes::BOUND, write_dest | flags_update | op1t(reg, read) | op2r(read) | size16) }, // ??

/* BSF */ { 0x0FBC, Inst_2(ISA::Opcodes::BSF, write_dest | flags_update | op1t(reg, read) | op2r(read)) },
/* BSR */ { 0x0FBD, Inst_2(ISA::Opcodes::BSR, write_dest | flags_update | op1t(reg, read) | op2r(read)) },

/* BT  */ { 0x0FA3, Inst_2(ISA::Opcodes::BT, flags_update | op1r(read) | op2t(reg, read)) },
		  { 0x0FBA OPEXT(4), Inst_2(ISA::Opcodes::BT, flags_update | op1r(read) | op2t(imm, read)) },
		  { 0x0FBA OPEXT(4), Inst_2(ISA::Opcodes::BT, flags_update | op1r(read) | op2t(imm, read)) },
			
/* BTC */ { 0x0FBB, Inst_2(ISA::Opcodes::BTC, write_dest | flags_update | op1r(read) | op2t(reg, read)) },
		  { 0x0FBA OPEXT(7), Inst_2(ISA::Opcodes::BTC, write_dest | flags_update | op1r(read) | op2t(imm, read)) },
		  { 0x0FBA OPEXT(7), Inst_2(ISA::Opcodes::BTC, write_dest | flags_update | op1r(read) | op2t(imm, read)) },
		  
/* BTR */ { 0x0FB3, Inst_2(ISA::Opcodes::BTR, write_dest | flags_update | op1r(read) | op2t(reg, read)) },
		  { 0x0FBA OPEXT(6), Inst_2(ISA::Opcodes::BTR, write_dest | flags_update | op1r(read) | op2t(imm, read)) },
		  { 0x0FBA OPEXT(6), Inst_2(ISA::Opcodes::BTR, write_dest | flags_update | op1r(read) | op2t(imm, read)) },
		  
/* BTS */ { 0x0FAB, Inst_2(ISA::Opcodes::BTS, write_dest | flags_update | op1r(read) | op2t(reg, read)) },
		  { 0x0FBA OPEXT(5), Inst_2(ISA::Opcodes::BTS, write_dest | flags_update | op1r(read) | op2t(imm, read)) },
		  { 0x0FBA OPEXT(5), Inst_2(ISA::Opcodes::BTS, write_dest | flags_update | op1r(read) | op2t(imm, read)) },
		  
/* CALL */	{ 0x9A, Inst_2(ISA::Opcodes::CALL, flags_update | op1t(imm, read)) }, // hacks
			{ 0xE8, Inst_2(ISA::Opcodes::CALL, op1t(imm, read)) },
			{ 0xFF OPEXT(2), Inst_2(ISA::Opcodes::CALL, op1r(read)) },
			{ 0xFF OPEXT(5), Inst_2(ISA::Opcodes::CALL, op1t(imm, read)) },
			
/* CBW */	{ 0x98, Inst_2(ISA::Opcodes::CBW, write_dest | op1t(reg, read) | reg1(reg_A)) },
/* CLC */	{ 0xF8, Inst_2(ISA::Opcodes::CLC, flags_update) },
/* CLD */	{ 0xFC, Inst_2(ISA::Opcodes::CLD, flags_update) },
/* CLI */	{ 0xFA, Inst_2(ISA::Opcodes::CLI, flags_update) },
/* CLTS */{ 0x0F06, Inst_2(ISA::Opcodes::CLTS, 0) },
/* CMC */	{ 0xF5, Inst_2(ISA::Opcodes::CMC, flags_update) },

/* CMP */	{ 0x38, Inst_2(ISA::Opcodes::CMP, flags_update | op1r(read) | op2t(reg, read) | size8) },
			{ 0x39, Inst_2(ISA::Opcodes::CMP, flags_update | op1r(read) | op2t(reg, read)) },
			{ 0x3A, Inst_2(ISA::Opcodes::CMP, flags_update | op1t(reg, read) | op2r(read) | size8) },
			{ 0x3B, Inst_2(ISA::Opcodes::CMP, flags_update | op1t(reg, read) | op2r(read)) },
			{ 0x3C, Inst_2(ISA::Opcodes::CMP, flags_update | op1t(reg, read) | op2t(imm, read) | reg1(reg_A) | size8) },
			{ 0x3C, Inst_2(ISA::Opcodes::CMP, flags_update | op1t(reg, read) | op2t(imm, read) | reg1(reg_A)) },
			{ 0x80 OPEXT(7), Inst_2(ISA::Opcodes::CMP, flags_update | op1r(read) | op2t(imm, read) | size8) },
			{ 0x81 OPEXT(7), Inst_2(ISA::Opcodes::CMP, flags_update | op1r(read) | op2t(imm, read)) },
			{ 0x83 OPEXT(7), Inst_2(ISA::Opcodes::CMP, flags_update | op1r(read) | op2t(imm, read)) },
			
/* CMPS */	{ 0xA6, Inst_2(ISA::Opcodes::CMPS, flags_update) },
			{ 0xA7, Inst_2(ISA::Opcodes::CMPS, flags_update) },
			
/* CWD */	{ 0x99, Inst_2(ISA::Opcodes::CWD, op1t(reg, read) | reg1(reg_A)) },
/* DAA */	{ 0x27, Inst_2(ISA::Opcodes::DAA, flags_update | op1t(reg, read) | reg1(reg_A) | size8) },
/* DAS */	{ 0x2F, Inst_2(ISA::Opcodes::DAS, flags_update | op1t(reg, read) | reg1(reg_A) | size8) },

/* DEC */	{ 0x48, Inst_2(ISA::Opcodes::DEC, write_dest | flags_update | op1t(reg, read)) },
			{ 0xFE OPEXT(1), Inst_2(ISA::Opcodes::DEC, write_dest | flags_update | op1r(read) | size8) },
			{ 0xFF OPEXT(1), Inst_2(ISA::Opcodes::DEC, write_dest | flags_update | op1r(read)) },
			
/* DIV */	{ 0xF6 OPEXT(6), Inst_2(ISA::Opcodes::DIV, flags_update | op1t(reg, read) | op2r(read) | reg1(reg_A) | out_reg(ISA::Registers::AX) | size8) },
			{ 0xF7 OPEXT(6), Inst_2(ISA::Opcodes::DIV, flags_update | op1t(reg, read) | op2r(read) | reg1(reg_A) | out_reg(ISA::Registers::EAX)) },
			
/* ENTER */ { 0xC8, Inst_2(ISA::Opcodes::ENTER, op1t(imm, read)) }, // all immediates are merged
/* HLT */   { 0xF4, Inst_2(ISA::Opcodes::HLT, 0) },

/* IDIV */  { 0xF6 OPEXT(7), Inst_2(ISA::Opcodes::IDIV, flags_update | op1r(read) | out_reg(ISA::Registers::AX) | size8) },
			{ 0xF7 OPEXT(7), Inst_2(ISA::Opcodes::IDIV, flags_update | op1r(read) | out_reg(ISA::Registers::EAX)) },
			
/* IMUL */  { 0xF6 OPEXT(5), Inst_2(ISA::Opcodes::IMUL, flags_update | op1r(read) | out_reg(ISA::Registers::AX) | size8) },
			{ 0xF7 OPEXT(5), Inst_2(ISA::Opcodes::IMUL, flags_update | op1r(read) | out_reg(ISA::Registers::EAX)) },
		  { 0x0FAF, Inst_2(ISA::Opcodes::IMUL, write_dest | flags_update | op1t(reg, read) | op2r(read)) },
			{ 0x6B, Inst_2(ISA::Opcodes::IMUL, write_dest | flags_update | op1t(reg, read) | op2r(read)) }, // + sign extended 8 bit immediate value, memory operand is optional ??
			{ 0x69, Inst_2(ISA::Opcodes::IMUL, write_dest | flags_update | op1t(reg, read) | op2r(read)) }, // + sign extended 16/32 bit immediate value, memory operand is optional ??
	};
#endif
}
