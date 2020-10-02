
#include "instructions.h"


Inst::Inst(U8 opcode, U32 flags, U32 address_value, U32 immediate_value)
	: opcode(opcode), address_value(address_value), immediate_value(immediate_value)
{
	// TODO : fix
	address_size_override = (flags >> 23) & 1;
	operand_size_override = (flags >> 22) & 1;
	address_byte_size_override = (flags >> 21) & 1;
	operand_byte_size_override = (flags >> 20) & 1;
	write_to_dest = (flags >> 19) & 1;
	register_out_override = (flags >> 18) & 1;
	register_out = (flags >> 13) & 0b11111;
	get_flags = (flags >> 12) & 1;
	op1_type = OpType((flags >> 10) & 0b11);
	op2_type = OpType((flags >> 8) & 0b11);
	read_op1 = (flags >> 7) & 1;
	read_op2 = (flags >> 6) & 1;
	op1_register = (flags >> 3) & 0b111;
	op2_register = (flags >> 0) & 0b111;
}


InstData Inst::getInstData() const
{
	return InstData{
		0, 0, 0,
		address_value,
		immediate_value,
		OpSize::DW, OpSize::DW, OpSize::DW
	};
}
