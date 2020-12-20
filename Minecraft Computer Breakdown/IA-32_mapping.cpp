
#include "IA-32_mapping.h"
#include "registers.h"

// TODO: fix macros, fix offsets in macros, automate the process from a table

#define size16 (0b11 << 22) // sets the operand and address sizes to two bytes
#define size8 (0b11 << 20) // sets the operand and address sizes to one byte
#define write_dest (0b1 << 19) // sets write to dest
#define out_override(reg) (0b1 << 18) | (reg << 13) // sets the register override
#define get_flags (0b1 << 12) // read and/or write flags

// operands specification
#define op1r(read) (read << 7)
#define op2r(read) (read << 6)
#define op1t(type, read) ((U8(type) << 10) | op1r(read))
#define op2t(type, read) ((U8(type) << 8) | op2r(read))
#define reg1(reg) (reg << 3) // register operands
#define reg2(reg) (reg << 0)

// operand types
#define reg OpType::REG
#define mem OpType::MEM
#define imm OpType::IMM

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


const std::map<U16, Inst> IA32::InstTable {
/* AAA */	{ 0x37, Inst(Opcodes::AAA, write_dest | get_flags | size16 | reg1(reg_A) | op1t(reg, read)) },
/* AAD */ { 0xD50A, Inst(Opcodes::AAD, write_dest | get_flags | size16 | reg1(reg_A) | op1t(reg, read)) },
/* AAM */ { 0xD40A, Inst(Opcodes::AAM, write_dest | get_flags | size16 | reg1(reg_A) | op1t(reg, read)) },
/* AAS */	{ 0x3F, Inst(Opcodes::AAS, write_dest | get_flags | size16 | reg1(reg_A) | op1t(reg, read)) },

/* ADC */	{ 0x10, Inst(Opcodes::ADC, write_dest | get_flags | op1r(read) | op2t(reg, read) | size8) },
			{ 0x11, Inst(Opcodes::ADC, write_dest | get_flags | op1r(read) | op2t(reg, read)) },
			{ 0x12, Inst(Opcodes::ADC, write_dest | get_flags | op1t(reg, read) | op2r(read) | size8) },
			{ 0x13, Inst(Opcodes::ADC, write_dest | get_flags | op1t(reg, read) | op2r(read)) },
			{ 0x14, Inst(Opcodes::ADC, write_dest | get_flags | op1t(reg, read) | op2t(imm, read) | reg1(reg_A) | size8) },
			{ 0x15, Inst(Opcodes::ADC, write_dest | get_flags | op1t(reg, read) | op2t(imm, read) | reg1(reg_A)) },
			{ 0x80 OPEXT(2), Inst(Opcodes::ADC, write_dest | get_flags | op1r(read) | op2t(imm, read) | size8) },
			{ 0x81 OPEXT(2), Inst(Opcodes::ADC, write_dest | get_flags | op1r(read) | op2t(imm, read)) },
			{ 0x83 OPEXT(2), Inst(Opcodes::ADC, write_dest | get_flags | op1r(read) | op2t(imm, read)) },
		
/* ADD */	{ 0x00, Inst(Opcodes::ADD, write_dest | get_flags | op1r(read) | op2t(reg, read) | size8) },
			{ 0x01, Inst(Opcodes::ADD, write_dest | get_flags | op1r(read) | op2t(reg, read)) },
			{ 0x02, Inst(Opcodes::ADD, write_dest | get_flags | op1t(reg, read) | op2r(read) | size8) },
			{ 0x03, Inst(Opcodes::ADD, write_dest | get_flags | op1t(reg, read) | op2r(read)) },
			{ 0x04, Inst(Opcodes::ADD, write_dest | get_flags | reg1(reg_A) | op2t(imm, read) | size8) },
			{ 0x05, Inst(Opcodes::ADD, write_dest | get_flags | reg1(reg_A) | op2t(imm, read)) },
			{ 0x80 OPEXT(0), Inst(Opcodes::ADD, write_dest | get_flags | op1r(read) | op2t(imm, read) | size8) },
			{ 0x81 OPEXT(0), Inst(Opcodes::ADD, write_dest | get_flags | op1r(read) | op2t(imm, read)) },
			{ 0x83 OPEXT(0), Inst(Opcodes::ADD, write_dest | get_flags | op1r(read) | op2t(imm, read)) },
			
/* AND */	{ 0x20, Inst(Opcodes::AND, write_dest | get_flags | op1r(read) | op2t(reg, read) | size8) },
			{ 0x21, Inst(Opcodes::AND, write_dest | get_flags | op1r(read) | op2t(reg, read)) },
			{ 0x22, Inst(Opcodes::AND, write_dest | get_flags | op1t(reg, read) | op2r(read) | size8) },
			{ 0x23, Inst(Opcodes::AND, write_dest | get_flags | op1t(reg, read) | op2r(read)) },
			{ 0x24, Inst(Opcodes::AND, write_dest | get_flags | op1t(reg, read) | op2t(imm, read) | reg1(reg_A) | size8) },
			{ 0x25, Inst(Opcodes::AND, write_dest | get_flags | op1t(reg, read) | op2t(imm, read) | reg1(reg_A)) },
			{ 0x80 OPEXT(4), Inst(Opcodes::AND, write_dest | get_flags | op1r(read) | op2t(imm, read) | size8) },
			{ 0x81 OPEXT(4), Inst(Opcodes::AND, write_dest | get_flags | op1r(read) | op2t(imm, read)) },
			{ 0x83 OPEXT(4), Inst(Opcodes::AND, write_dest | get_flags | op1r(read) | op2t(imm, read)) },
			
/* ARPL */	{ 0x63, Inst(Opcodes::ARPL, write_dest | get_flags | op1r(read) | op2t(reg, read) | size16) },
/* BOUND */	{ 0x62, Inst(Opcodes::BOUND, op1t(reg, read) | op2t(mem, read)) }, // problem: we must read at the memory pointer of the second operand, but also at the next one

/* BSF */ { 0x0FBC, Inst(Opcodes::BSF, write_dest | get_flags | op1t(reg, read) | op2r(read)) },
/* BSR */ { 0x0FBD, Inst(Opcodes::BSR, write_dest | get_flags | op1t(reg, read) | op2r(read)) },

/* BT  */ { 0x0FA3, Inst(Opcodes::BT, get_flags | op1r(read) | op2t(reg, read)) },
		  { 0x0FBA OPEXT(4), Inst(Opcodes::BT, get_flags | op1r(read) | op2t(imm, read)) },
		  { 0x0FBA OPEXT(4), Inst(Opcodes::BT, get_flags | op1r(read) | op2t(imm, read)) },
			
/* BTC */ { 0x0FBB, Inst(Opcodes::BTC, write_dest | get_flags | op1r(read) | op2t(reg, read)) },
		  { 0x0FBA OPEXT(7), Inst(Opcodes::BTC, write_dest | get_flags | op1r(read) | op2t(imm, read)) },
		  { 0x0FBA OPEXT(7), Inst(Opcodes::BTC, write_dest | get_flags | op1r(read) | op2t(imm, read)) },
		  
/* BTR */ { 0x0FB3, Inst(Opcodes::BTR, write_dest | get_flags | op1r(read) | op2t(reg, read)) },
		  { 0x0FBA OPEXT(6), Inst(Opcodes::BTR, write_dest | get_flags | op1r(read) | op2t(imm, read)) },
		  { 0x0FBA OPEXT(6), Inst(Opcodes::BTR, write_dest | get_flags | op1r(read) | op2t(imm, read)) },
		  
/* BTS */ { 0x0FAB, Inst(Opcodes::BTS, write_dest | get_flags | op1r(read) | op2t(reg, read)) },
		  { 0x0FBA OPEXT(5), Inst(Opcodes::BTS, write_dest | get_flags | op1r(read) | op2t(imm, read)) },
		  { 0x0FBA OPEXT(5), Inst(Opcodes::BTS, write_dest | get_flags | op1r(read) | op2t(imm, read)) },
		  
/* CALL */	{ 0x9A, Inst(Opcodes::CALL, op1t(imm, read)) }, // hacks
			{ 0xE8, Inst(Opcodes::CALL, op1t(imm, read)) },
			{ 0xFF OPEXT(2), Inst(Opcodes::CALL, op1r(read)) },
			{ 0xFF OPEXT(5), Inst(Opcodes::CALL, op1t(imm, read)) },
			
/* CBW */	{ 0x98, Inst(Opcodes::CBW, write_dest | op1t(reg, read) | reg1(reg_A)) },
/* CLC */	{ 0xF8, Inst(Opcodes::CLC, get_flags) },
/* CLD */	{ 0xFC, Inst(Opcodes::CLD, get_flags) },
/* CLI */	{ 0xFA, Inst(Opcodes::CLI, get_flags) },
/* CLTS */{ 0x0F06, Inst(Opcodes::CLTS, 0) },
/* CMC */	{ 0xF5, Inst(Opcodes::CMC, get_flags) },

/* CMP */	{ 0x38, Inst(Opcodes::CMP, get_flags | op1r(read) | op2t(reg, read) | size8) },
			{ 0x39, Inst(Opcodes::CMP, get_flags | op1r(read) | op2t(reg, read)) },
			{ 0x3A, Inst(Opcodes::CMP, get_flags | op1t(reg, read) | op2r(read) | size8) },
			{ 0x3B, Inst(Opcodes::CMP, get_flags | op1t(reg, read) | op2r(read)) },
			{ 0x3C, Inst(Opcodes::CMP, get_flags | op1t(reg, read) | op2t(imm, read) | reg1(reg_A) | size8) },
			{ 0x3D, Inst(Opcodes::CMP, get_flags | op1t(reg, read) | op2t(imm, read) | reg1(reg_A)) },
			{ 0x80 OPEXT(7), Inst(Opcodes::CMP, get_flags | op1r(read) | op2t(imm, read) | size8) },
			{ 0x81 OPEXT(7), Inst(Opcodes::CMP, get_flags | op1r(read) | op2t(imm, read)) },
			{ 0x83 OPEXT(7), Inst(Opcodes::CMP, get_flags | op1r(read) | op2t(imm, read)) },  // special treatment for this instruction: the immediate should the sign extended
			
/* CMPS */	{ 0xA6, Inst(Opcodes::CMPS, get_flags) },
			{ 0xA7, Inst(Opcodes::CMPS, get_flags) },
			
/* CWD */	{ 0x99, Inst(Opcodes::CWD, op1t(reg, no_read) | op2t(reg, read) | reg1(reg_D) | reg2(reg_A)) },
/* DAA */	{ 0x27, Inst(Opcodes::DAA, get_flags | op1t(reg, read) | reg1(reg_A) | size8) },
/* DAS */	{ 0x2F, Inst(Opcodes::DAS, get_flags | op1t(reg, read) | reg1(reg_A) | size8) },

/* DEC */	{ 0x48, Inst(Opcodes::DEC, write_dest | get_flags | op1t(reg, read)) },
			{ 0xFE OPEXT(1), Inst(Opcodes::DEC, write_dest | get_flags | op1r(read) | size8) },
			{ 0xFF OPEXT(1), Inst(Opcodes::DEC, write_dest | get_flags | op1r(read)) },
			
/* DIV */	{ 0xF6 OPEXT(6), Inst(Opcodes::DIV, op1t(reg, read) | op2r(read) | reg1(reg_A) | out_reg(Register::AX) | size8) },
			{ 0xF7 OPEXT(6), Inst(Opcodes::DIV, op1t(reg, read) | op2r(read) | reg1(reg_A) | out_reg(Register::EAX)) },
			
/* ENTER */ { 0xC8, Inst(Opcodes::ENTER, op1t(imm, read)) }, // all immediates are merged
/* HLT */   { 0xF4, Inst(Opcodes::HLT, 0) },

/* IDIV */  { 0xF6 OPEXT(7), Inst(Opcodes::IDIV, op1r(read) | out_reg(Register::AX) | size8) },
			{ 0xF7 OPEXT(7), Inst(Opcodes::IDIV, op1r(read) | out_reg(Register::EAX)) },
			
/* IMUL */  { 0xF6 OPEXT(5), Inst(Opcodes::IMUL, get_flags | op1r(read) | out_reg(Register::AX) | size8) },
			{ 0xF7 OPEXT(5), Inst(Opcodes::IMUL, get_flags | op1r(read) | out_reg(Register::EAX)) },
		  { 0x0FAF, Inst(Opcodes::IMUL, write_dest | get_flags | op1t(reg, read) | op2r(read)) },
			{ 0x6B, Inst(Opcodes::IMUL, write_dest | get_flags | op1t(reg, read) | op2r(read)) }, // + sign extended 8 bit immediate value, memory operand is optional ??
			{ 0x69, Inst(Opcodes::IMUL, write_dest | get_flags | op1t(reg, read) | op2r(read)) }, // + sign extended 16/32 bit immediate value, memory operand is optional ??
};
