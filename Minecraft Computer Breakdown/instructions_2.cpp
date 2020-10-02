
#include "instructions_2.hpp"

/*
#define arithmethic 0
#define not_arithmethic (1 << 8)
#define jmp (1 << 7)
#define str (1 << 6)

static int op_count = 0;

namespace Opcodes_2
{
	constexpr U8 AAA	 = (op_count++) | arithmethic;
	constexpr U8 AAD     = (op_count++) | arithmethic;
	const U8 AAM     = (op_count++) | arithmethic;
	const U8 AAS     = (op_count++) | arithmethic;
	const U8 ADC     = (op_count++) | arithmethic;
	const U8 ADD     = (op_count++) | arithmethic;
	const U8 AND     = (op_count++) | arithmethic;
	const U8 ARPL    = (op_count++) | arithmethic;
	
	const U8 BSF     = (op_count++) | arithmethic;
	const U8 BSR     = (op_count++) | arithmethic;
	const U8 BT      = (op_count++) | arithmethic;
	const U8 BTC     = (op_count++) | arithmethic;
	const U8 BTR     = (op_count++) | arithmethic;
	const U8 BTS     = (op_count++) | arithmethic;
	
	const U8 CBW     = (op_count++) | arithmethic;
	const U8 CLC     = (op_count++) | arithmethic;
	const U8 CLD     = (op_count++) | arithmethic;
	const U8 CLI     = (op_count++) | arithmethic;
	const U8 CLTS    = (op_count++) | arithmethic;
	const U8 CMC     = (op_count++) | arithmethic;
	const U8 CMP     = (op_count++) | arithmethic;
	
	const U8 CWD     = (op_count++) | arithmethic;
	const U8 DAA     = (op_count++) | arithmethic;
	const U8 DAS     = (op_count++) | arithmethic;
	const U8 DEC     = (op_count++) | arithmethic;
	const U8 DIV     = (op_count++) | arithmethic;
	
	const U8 IDIV    = (op_count++) | arithmethic;
	const U8 IMUL    = (op_count++) | arithmethic;
	
	const U8 INC     = (op_count++) | arithmethic;
	
	const U8 LEA     = (op_count++) | arithmethic;
	
	const U8 MOV     = (op_count++) | arithmethic;
	
	const U8 MOVSX   = (op_count++) | arithmethic;
	const U8 MOVZX   = (op_count++) | arithmethic;
	const U8 MUL     = (op_count++) | arithmethic;
	const U8 NEG     = (op_count++) | arithmethic;
	const U8 NOP     = (op_count++) | arithmethic;
	const U8 NOT     = (op_count++) | arithmethic;
	const U8 OR      = (op_count++) | arithmethic;
	
	const U8 RCL     = (op_count++) | arithmethic;
	
	const U8 SAL     = (op_count++) | arithmethic;
	const U8 SBB     = (op_count++) | arithmethic;
	
	const U8 SETcc   = (op_count++) | arithmethic;
	
	const U8 SHRD    = (op_count++) | arithmethic;
	const U8 SLDT    = (op_count++) | arithmethic;
	
	const U8 STC     = (op_count++) | arithmethic;
	const U8 STD     = (op_count++) | arithmethic;
	const U8 STI     = (op_count++) | arithmethic;
	
	const U8 SUB     = (op_count++) | arithmethic;
	const U8 TEST    = (op_count++) | arithmethic;
	
	const U8 XCHG    = (op_count++) | arithmethic;
	const U8 XLAT    = (op_count++) | arithmethic;
	const U8 XOR     = (op_count++) | arithmethic;
	
	// Non arithmetic only instructions
	
	const U8 BOUND   = (op_count = 0, (op_count++) | not_arithmethic); // hacky way to reset 'op_count'
	
	const U8 CALL    = (op_count++) | not_arithmethic | jmp;
	
	const U8 CMPS    = (op_count++) | not_arithmethic | str;
	
	const U8 ENTER   = (op_count++) | not_arithmethic;
	const U8 HLT     = (op_count++) | not_arithmethic;
	
	const U8 IN      = (op_count++) | not_arithmethic;
	
	const U8 INS     = (op_count++) | not_arithmethic | str;
	const U8 INT     = (op_count++) | not_arithmethic | jmp;
	const U8 IRET    = (op_count++) | not_arithmethic | jmp;
	const U8 Jcc     = (op_count++) | not_arithmethic | jmp;
	const U8 JMP     = (op_count++) | not_arithmethic | jmp;
	const U8 LAHF    = (op_count++) | not_arithmethic;
	const U8 LAR     = (op_count++) | not_arithmethic;
	
	const U8 LEAVE   = (op_count++) | not_arithmethic | jmp;
	const U8 LGDT    = (op_count++) | not_arithmethic;
	const U8 LGS     = (op_count++) | not_arithmethic;
	const U8 LLDT    = (op_count++) | not_arithmethic;
	const U8 LMSW    = (op_count++) | not_arithmethic;
	const U8 LOCK    = (op_count++) | not_arithmethic;
	const U8 LODS    = (op_count++) | not_arithmethic | str;
	const U8 LOOP    = (op_count++) | not_arithmethic | jmp;
	const U8 LSL     = (op_count++) | not_arithmethic;
	const U8 LTR     = (op_count++) | not_arithmethic;
	
	const U8 MOVS    = (op_count++) | not_arithmethic | str;
	
	const U8 OUT     = (op_count++) | not_arithmethic;
	const U8 OUTS    = (op_count++) | not_arithmethic | str;
	const U8 POP     = (op_count++) | not_arithmethic;
	const U8 POPA    = (op_count++) | not_arithmethic;
	const U8 POPF    = (op_count++) | not_arithmethic;
	const U8 PUSH    = (op_count++) | not_arithmethic;
	const U8 PUSHA   = (op_count++) | not_arithmethic;
	const U8 PUSHF   = (op_count++) | not_arithmethic;
	
	const U8 REP     = (op_count++) | not_arithmethic | jmp | str;
	const U8 RET     = (op_count++) | not_arithmethic | jmp;
	const U8 SAHF    = (op_count++) | not_arithmethic;
	
	const U8 SCAS    = (op_count++) | not_arithmethic | str;
	
	const U8 SGDT    = (op_count++) | not_arithmethic;
	
	const U8 SMSW    = (op_count++) | not_arithmethic;
	
	const U8 STOS    = (op_count++) | not_arithmethic | str;
	const U8 STR     = (op_count++) | not_arithmethic;
	
	const U8 VERR    = (op_count++) | not_arithmethic;
	const U8 WAIT    = (op_count++) | not_arithmethic;
};
*/


Inst_2::Inst_2(U16 opcode,  U32 flags, U32 address_value, U32 immediate_value)
	: opcode(opcode), address_value(address_value), immediate_value(immediate_value)
{
	address_size_override = (flags >> 23) & 1;
	operand_size_override = (flags >> 22) & 1;
	address_byte_size_override = (flags >> 21) & 1;
	operand_byte_size_override = (flags >> 20) & 1;
	write_to_dest = (flags >> 19) & 1;
	register_out_override = (flags >> 18) & 1;
	register_out = (flags >> 13) & 0b11111;
	get_flags = (flags >> 12) & 1;
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


#define size16 (0b11 << 22) // sets the operand and address sizes to two bytes
#define size8 (0b11 << 20) // sets the operand and address sizes to one byte
#define write_dest (0b1 << 19) // sets write to dest
#define out_override(reg) (0b1 << 18) | (reg << 13) // sets the register override
#define get_flags (0b1 << 12) // read and/or write flags
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
#define reg Inst_2::REG
#define mem Inst_2::MEM
#define imm Inst_2::IMM

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

const std::map<U16, Inst_2> ISA::InstTable {
/* AAA */	{ 0x37, Inst_2(ISA::Opcodes::AAA, write_dest | get_flags | size16 | reg1(reg_A) | op1t(reg, read)) },
/* AAD */ { 0xD50A, Inst_2(ISA::Opcodes::AAD, write_dest | get_flags | size16 | reg1(reg_A) | op1t(reg, read)) },
/* AAM */ { 0xD40A, Inst_2(ISA::Opcodes::AAM, write_dest | get_flags | size16 | reg1(reg_A) | op1t(reg, read)) },
/* AAS */	{ 0x3F, Inst_2(ISA::Opcodes::AAS, write_dest | get_flags | size16 | reg1(reg_A) | op1t(reg, read)) },

/* ADC */	{ 0x10, Inst_2(ISA::Opcodes::ADC, write_dest | get_flags | op1r(read) | op2t(reg, read) | size8) },
			{ 0x11, Inst_2(ISA::Opcodes::ADC, write_dest | get_flags | op1r(read) | op2t(reg, read)) },
			{ 0x12, Inst_2(ISA::Opcodes::ADC, write_dest | get_flags | op1t(reg, read) | op2r(read) | size8) },
			{ 0x13, Inst_2(ISA::Opcodes::ADC, write_dest | get_flags | op1t(reg, read) | op2r(read)) },
			{ 0x14, Inst_2(ISA::Opcodes::ADC, write_dest | get_flags | op1t(reg, read) | op2t(imm, read) | reg1(reg_A) | size8) },
			{ 0x15, Inst_2(ISA::Opcodes::ADC, write_dest | get_flags | op1t(reg, read) | op2t(imm, read) | reg1(reg_A)) },
			{ 0x80 OPEXT(2), Inst_2(ISA::Opcodes::ADC, write_dest | get_flags | op1r(read) | op2t(imm, read) | size8) },
			{ 0x81 OPEXT(2), Inst_2(ISA::Opcodes::ADC, write_dest | get_flags | op1r(read) | op2t(imm, read)) },
			{ 0x83 OPEXT(2), Inst_2(ISA::Opcodes::ADC, write_dest | get_flags | op1r(read) | op2t(imm, read)) },
		
/* ADD */	{ 0x00, Inst_2(ISA::Opcodes::ADD, write_dest | get_flags | op1r(read) | op2t(reg, read) | size8) },
			{ 0x01, Inst_2(ISA::Opcodes::ADD, write_dest | get_flags | op1r(read) | op2t(reg, read)) },
			{ 0x02, Inst_2(ISA::Opcodes::ADD, write_dest | get_flags | op1t(reg, read) | op2r(read) | size8) },
			{ 0x03, Inst_2(ISA::Opcodes::ADD, write_dest | get_flags | op1t(reg, read) | op2r(read)) },
			{ 0x04, Inst_2(ISA::Opcodes::ADD, write_dest | get_flags | reg1(reg_A) | op2t(imm, read) | size8) },
			{ 0x05, Inst_2(ISA::Opcodes::ADD, write_dest | get_flags | reg1(reg_A) | op2t(imm, read)) },
			{ 0x80 OPEXT(0), Inst_2(ISA::Opcodes::ADD, write_dest | get_flags | op1r(read) | op2t(imm, read) | size8) },
			{ 0x81 OPEXT(0), Inst_2(ISA::Opcodes::ADD, write_dest | get_flags | op1r(read) | op2t(imm, read)) },
			{ 0x83 OPEXT(0), Inst_2(ISA::Opcodes::ADD, write_dest | get_flags | op1r(read) | op2t(imm, read)) },
			
/* AND */	{ 0x20, Inst_2(ISA::Opcodes::AND, write_dest | get_flags | op1r(read) | op2t(reg, read) | size8) },
			{ 0x21, Inst_2(ISA::Opcodes::AND, write_dest | get_flags | op1r(read) | op2t(reg, read)) },
			{ 0x22, Inst_2(ISA::Opcodes::AND, write_dest | get_flags | op1t(reg, read) | op2r(read) | size8) },
			{ 0x23, Inst_2(ISA::Opcodes::AND, write_dest | get_flags | op1t(reg, read) | op2r(read)) },
			{ 0x24, Inst_2(ISA::Opcodes::AND, write_dest | get_flags | op1t(reg, read) | op2t(imm, read) | reg1(reg_A) | size8) },
			{ 0x25, Inst_2(ISA::Opcodes::AND, write_dest | get_flags | op1t(reg, read) | op2t(imm, read) | reg1(reg_A)) },
			{ 0x80 OPEXT(4), Inst_2(ISA::Opcodes::AND, write_dest | get_flags | op1r(read) | op2t(imm, read) | size8) },
			{ 0x81 OPEXT(4), Inst_2(ISA::Opcodes::AND, write_dest | get_flags | op1r(read) | op2t(imm, read)) },
			{ 0x83 OPEXT(4), Inst_2(ISA::Opcodes::AND, write_dest | get_flags | op1r(read) | op2t(imm, read)) },
			
/* ARPL */	{ 0x63, Inst_2(ISA::Opcodes::ARPL, write_dest | get_flags | op1r(read) | op2t(reg, read) | size16) },
/* BOUND */	{ 0x62, Inst_2(ISA::Opcodes::BOUND, op1t(reg, read) | op2t(mem, read)) }, // problem: we must read at the memory pointer of the second operand, but also at the next one

/* BSF */ { 0x0FBC, Inst_2(ISA::Opcodes::BSF, write_dest | get_flags | op1t(reg, read) | op2r(read)) },
/* BSR */ { 0x0FBD, Inst_2(ISA::Opcodes::BSR, write_dest | get_flags | op1t(reg, read) | op2r(read)) },

/* BT  */ { 0x0FA3, Inst_2(ISA::Opcodes::BT, get_flags | op1r(read) | op2t(reg, read)) },
		  { 0x0FBA OPEXT(4), Inst_2(ISA::Opcodes::BT, get_flags | op1r(read) | op2t(imm, read)) },
		  { 0x0FBA OPEXT(4), Inst_2(ISA::Opcodes::BT, get_flags | op1r(read) | op2t(imm, read)) },
			
/* BTC */ { 0x0FBB, Inst_2(ISA::Opcodes::BTC, write_dest | get_flags | op1r(read) | op2t(reg, read)) },
		  { 0x0FBA OPEXT(7), Inst_2(ISA::Opcodes::BTC, write_dest | get_flags | op1r(read) | op2t(imm, read)) },
		  { 0x0FBA OPEXT(7), Inst_2(ISA::Opcodes::BTC, write_dest | get_flags | op1r(read) | op2t(imm, read)) },
		  
/* BTR */ { 0x0FB3, Inst_2(ISA::Opcodes::BTR, write_dest | get_flags | op1r(read) | op2t(reg, read)) },
		  { 0x0FBA OPEXT(6), Inst_2(ISA::Opcodes::BTR, write_dest | get_flags | op1r(read) | op2t(imm, read)) },
		  { 0x0FBA OPEXT(6), Inst_2(ISA::Opcodes::BTR, write_dest | get_flags | op1r(read) | op2t(imm, read)) },
		  
/* BTS */ { 0x0FAB, Inst_2(ISA::Opcodes::BTS, write_dest | get_flags | op1r(read) | op2t(reg, read)) },
		  { 0x0FBA OPEXT(5), Inst_2(ISA::Opcodes::BTS, write_dest | get_flags | op1r(read) | op2t(imm, read)) },
		  { 0x0FBA OPEXT(5), Inst_2(ISA::Opcodes::BTS, write_dest | get_flags | op1r(read) | op2t(imm, read)) },
		  
/* CALL */	{ 0x9A, Inst_2(ISA::Opcodes::CALL, op1t(imm, read)) }, // hacks
			{ 0xE8, Inst_2(ISA::Opcodes::CALL, op1t(imm, read)) },
			{ 0xFF OPEXT(2), Inst_2(ISA::Opcodes::CALL, op1r(read)) },
			{ 0xFF OPEXT(5), Inst_2(ISA::Opcodes::CALL, op1t(imm, read)) },
			
/* CBW */	{ 0x98, Inst_2(ISA::Opcodes::CBW, write_dest | op1t(reg, read) | reg1(reg_A)) },
/* CLC */	{ 0xF8, Inst_2(ISA::Opcodes::CLC, get_flags) },
/* CLD */	{ 0xFC, Inst_2(ISA::Opcodes::CLD, get_flags) },
/* CLI */	{ 0xFA, Inst_2(ISA::Opcodes::CLI, get_flags) },
/* CLTS */{ 0x0F06, Inst_2(ISA::Opcodes::CLTS, 0) },
/* CMC */	{ 0xF5, Inst_2(ISA::Opcodes::CMC, get_flags) },

/* CMP */	{ 0x38, Inst_2(ISA::Opcodes::CMP, get_flags | op1r(read) | op2t(reg, read) | size8) },
			{ 0x39, Inst_2(ISA::Opcodes::CMP, get_flags | op1r(read) | op2t(reg, read)) },
			{ 0x3A, Inst_2(ISA::Opcodes::CMP, get_flags | op1t(reg, read) | op2r(read) | size8) },
			{ 0x3B, Inst_2(ISA::Opcodes::CMP, get_flags | op1t(reg, read) | op2r(read)) },
			{ 0x3C, Inst_2(ISA::Opcodes::CMP, get_flags | op1t(reg, read) | op2t(imm, read) | reg1(reg_A) | size8) },
			{ 0x3D, Inst_2(ISA::Opcodes::CMP, get_flags | op1t(reg, read) | op2t(imm, read) | reg1(reg_A)) },
			{ 0x80 OPEXT(7), Inst_2(ISA::Opcodes::CMP, get_flags | op1r(read) | op2t(imm, read) | size8) },
			{ 0x81 OPEXT(7), Inst_2(ISA::Opcodes::CMP, get_flags | op1r(read) | op2t(imm, read)) },
			{ 0x83 OPEXT(7), Inst_2(ISA::Opcodes::CMP, get_flags | op1r(read) | op2t(imm, read)) },  // special treatment for this instruction: the immediate should the sign extended
			
/* CMPS */	{ 0xA6, Inst_2(ISA::Opcodes::CMPS, get_flags) },
			{ 0xA7, Inst_2(ISA::Opcodes::CMPS, get_flags) },
			
/* CWD */	{ 0x99, Inst_2(ISA::Opcodes::CWD, op1t(reg, no_read) | op2t(reg, read) | reg1(reg_D) | reg2(reg_A)) },
/* DAA */	{ 0x27, Inst_2(ISA::Opcodes::DAA, get_flags | op1t(reg, read) | reg1(reg_A) | size8) },
/* DAS */	{ 0x2F, Inst_2(ISA::Opcodes::DAS, get_flags | op1t(reg, read) | reg1(reg_A) | size8) },

/* DEC */	{ 0x48, Inst_2(ISA::Opcodes::DEC, write_dest | get_flags | op1t(reg, read)) },
			{ 0xFE OPEXT(1), Inst_2(ISA::Opcodes::DEC, write_dest | get_flags | op1r(read) | size8) },
			{ 0xFF OPEXT(1), Inst_2(ISA::Opcodes::DEC, write_dest | get_flags | op1r(read)) },
			
/* DIV */	{ 0xF6 OPEXT(6), Inst_2(ISA::Opcodes::DIV, op1t(reg, read) | op2r(read) | reg1(reg_A) | out_reg(ISA::Registers::AX) | size8) },
			{ 0xF7 OPEXT(6), Inst_2(ISA::Opcodes::DIV, op1t(reg, read) | op2r(read) | reg1(reg_A) | out_reg(ISA::Registers::EAX)) },
			
/* ENTER */ { 0xC8, Inst_2(ISA::Opcodes::ENTER, op1t(imm, read)) }, // all immediates are merged
/* HLT */   { 0xF4, Inst_2(ISA::Opcodes::HLT, 0) },

/* IDIV */  { 0xF6 OPEXT(7), Inst_2(ISA::Opcodes::IDIV, op1r(read) | out_reg(ISA::Registers::AX) | size8) },
			{ 0xF7 OPEXT(7), Inst_2(ISA::Opcodes::IDIV, op1r(read) | out_reg(ISA::Registers::EAX)) },
			
/* IMUL */  { 0xF6 OPEXT(5), Inst_2(ISA::Opcodes::IMUL, get_flags | op1r(read) | out_reg(ISA::Registers::AX) | size8) },
			{ 0xF7 OPEXT(5), Inst_2(ISA::Opcodes::IMUL, get_flags | op1r(read) | out_reg(ISA::Registers::EAX)) },
		  { 0x0FAF, Inst_2(ISA::Opcodes::IMUL, write_dest | get_flags | op1t(reg, read) | op2r(read)) },
			{ 0x6B, Inst_2(ISA::Opcodes::IMUL, write_dest | get_flags | op1t(reg, read) | op2r(read)) }, // + sign extended 8 bit immediate value, memory operand is optional ??
			{ 0x69, Inst_2(ISA::Opcodes::IMUL, write_dest | get_flags | op1t(reg, read) | op2r(read)) }, // + sign extended 16/32 bit immediate value, memory operand is optional ??
};
