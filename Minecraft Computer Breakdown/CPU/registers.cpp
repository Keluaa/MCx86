
#include "registers.h"

#include "exceptions.h"


U32 Registers::read(const Register register_id) const
{
	U8 register_index = static_cast<U8>(register_id) & 0b111; // mod 8

	if (register_id <= Register::EDI) {
		return registers[register_index];
	}
	else if (register_id <= Register::DI) {
		return registers[register_index] & 0xFFFF;
	}
	else if (register_id <= Register::BL) {
		return registers[register_index] & 0xFF;
	}
	else if (register_id <= Register::BH) {
		return (registers[register_index] & 0xFF00) >> 8;
	}
	else if (register_id <= Register::GS) {
		return segments[register_index];
	}
	else if (register_id <= Register::CR1) {
		// We can tell apart CR0 and CR1 from their last bit
		return control_registers[register_index & 0b001];
	}
	else {
		throw RegisterException("Wrong register id", static_cast<U8>(register_id));
	}
}


U32 Registers::read(const Register register_id, OpSize size) const
{
	U8 register_index = static_cast<U8>(register_id) & 0b111; // mod 8
	return read_index(register_index, size);
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


void Registers::write(const Register register_id, const U32 new_value)
{
	U8 register_index = static_cast<U8>(register_id) % 8;

	if (register_id <= Register::EDI) {
		registers[register_index] = new_value;
	}
	else if (register_id <= Register::DI) {
		registers[register_index] = (registers[register_index] & 0xFFFF0000) | new_value;
	}
	else if (register_id <= Register::BL) {
		registers[register_index] = (registers[register_index] & 0xFFFFFF00) | new_value;
	}
	else if (register_id <= Register::BH) {
		registers[register_index] = (registers[register_index] & 0xFFFF00FF) | (new_value << 8);
	}
	else if (register_id <= Register::GS) {
		segments[register_index] = new_value;
	}
	else if (register_id <= Register::CR1) {
		throw RegisterException("Control registers cannot be set directly", static_cast<U8>(register_id));
	}
	else {
		throw RegisterException("Wrong register id", static_cast<U8>(register_id));
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
