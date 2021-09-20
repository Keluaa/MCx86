
#include "registers.h"


U32 Registers::read(const U8 register_id) const 
{
	if (register_id < 8) {
		return read_32(register_id);
	}
	else if (register_id < 16) {
		return read_16(register_id);
	}
	else if (register_id < 20) {
		return read_8_High(register_id);
	}
	else if (register_id < 24) {
		return read_8_Low(register_id);
	}
	else {
		throw RegisterException("Wrong register id", register_id);
	}
}


U32 Registers::read_index(U8 register_index, OpSize size) const
{
	switch (size)
	{
	case OpSize::B:
		if (register_index < 4) {
			// Low byte
			return registers[register_index] & 0xFF;
		} else {
			// High byte
			return (registers[register_index - 4] & 0xFF00) >> 8;
		}
	case OpSize::W:
		return registers[register_index] & 0xFFFF;
	case OpSize::DW:
		return registers[register_index];
	default:
		throw RegisterException("Wrong register size", register_index);
	}
}


void Registers::write(const U8 register_id, const U32 new_value, const U32 other_value)
{
	if (register_id < 8) {
		write_32(register_id, new_value);
	}
	else if (register_id < 16) {
		write_16(register_id, new_value);
	}
	else if (register_id < 20) {
		write_8_High(register_id, new_value);
	}
	else if (register_id < 24) {
		write_8_Low(register_id, new_value);
	}
	else if (register_id == 24) {
		// 64 bit assignment
		write_32(0, new_value);
		write_32(2, other_value);
	}
	else {
		throw RegisterException("Wrong register id", register_id);
	}
}


void Registers::write_index(U8 register_index, U32 value, OpSize size)
{
	switch (size)
	{
	case OpSize::B:
		if (register_index < 4) {
			// Low byte
			registers[register_index] |= value & 0xFF;
		}
		else {
			// High byte
			registers[register_index] |= (value & 0xFF) << 8;
		}
		break;
	case OpSize::W:
		registers[register_index] |= value & 0xFFFF;
		break;
	case OpSize::DW:
		registers[register_index] = value;
		break;
	default:
		throw RegisterException("Wrong register size", register_index);
	}
}


void Registers::reset_general_purpose_registers()
{
	std::memset(registers, 0, 8);
}


void Registers::reset_segments_registers()
{
	std::memset(segments, 0, 6);
}


void Registers::reset_control_registers()
{
	std::memset(control_registers, 0, 4);
}


void Registers::complete_reset()
{
	reset_general_purpose_registers();
	reset_segments_registers();
	reset_control_registers();

	EIP = 0;
	EFLAGS = 0b10;
	IDT_base = 0;
	IDT_limit = 0;
}
