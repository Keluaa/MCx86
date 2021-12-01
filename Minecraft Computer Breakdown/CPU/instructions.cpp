
#include "instructions.h"


Inst::Inst(U8 opcode, U32 flags, U32 address_value, U32 immediate_value)
	: opcode(opcode), 
	  raw_address_specifier(0),
	  address_value(address_value),
	  immediate_value(immediate_value)
{
	// TODO : fix offsets
	address_size_override = (flags >> 23) & 1;
	operand_size_override = (flags >> 22) & 1;
	address_byte_size_override = (flags >> 21) & 1;
	operand_byte_size_override = (flags >> 20) & 1;
	get_flags = (flags >> 12) & 1;
	op1_type = OpType((flags >> 10) & 0b11);
	op2_type = OpType((flags >> 8) & 0b11);
	op1_register = (flags >> 3) & 0b111;
	op2_register = (flags >> 0) & 0b111;
	read_op1 = (flags >> 7) & 1;
	read_op2 = (flags >> 6) & 1;
	write_ret1_to_op1 = (flags >> 0) & 1;
	write_ret2_to_op2 = (flags >> 0) & 1;
	write_ret1_to_register = (flags >> 0) & 1;
	scale_output_override = (flags >> 0) & 1;
	register_out = (flags >> 0) & 0b111;
}


InstData Inst::getInstData() const
{
	// All OpSizes are DW by default, and the address is 0
	return InstData{ .imm = immediate_value };
}
