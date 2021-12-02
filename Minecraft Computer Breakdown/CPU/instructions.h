#pragma once

#include "../data_types.h"


struct InstData
{
	U32 op1, op2, op3;
	
	U32 address;
	U32 imm;
	
	OpSize op1_size:2;
	OpSize op2_size:2;
	OpSize op3_size:2;
	U8:2; // padding

	// Used only for non-arithmetic instructions
	OpSize op_size:2;
	OpSize ad_size:2;
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
		- 1 bit: pre-compute address using the mod r/m, SIB and displacement bytes
		- 16 bits: mod r/m and SIB bytes
		- 32 bits: address value (or displacement)
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
	
	bit compute_address:1;
	
	U8:0; // alignment
	
	// Optional mod r/m and SIB bytes
	union {
		U16 raw_address_specifier;
		struct {
			// mod r/m byte
			U8 mod:2;
			U8 reg:3;
			U8 rm:3;
			
			// SIB byte
			U8 scale:2;
			U8 index:3;
			U8 base:3;
		} mod_rm_sib;
	};
	
	// both of those values can be used as general purpose values in spacial instructions (bound, call...)
	U32 address_value; 
	U32 immediate_value;

    Inst() = default;
	Inst(U8 opcode, U32 flags, U32 address_value = 0, U32 immediate_value = 0);
	
	[[nodiscard]] InstData getInstData() const;
};
