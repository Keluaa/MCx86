﻿
#include "registers.h"

#include "exceptions.h"

#include "../cycle_changes_monitor.h"


/**
 * Reads a register from its ID
 */
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


/**
 * Reads a scalable register (EAX, EDX, etc...) from its ID
 */
U32 Registers::read(const Register register_id, OpSize size) const
{
	U8 register_index = static_cast<U8>(register_id) & 0b111; // mod 8
	return read_index(register_index, size);
}


/**
 * Reads a scalable register (EAX, EDX, etc...)
 */
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


/**
 * Writes a register from its ID
 */
void Registers::write(const Register register_id, const U32 new_value)
{
    // TODO : separate those changes monitor from the other things
	U8 register_index = static_cast<U8>(register_id) % 8;

	if (register_id <= Register::EDI) {
        if (registers[register_index] != new_value) {
            register_change(register_id);
        }
		registers[register_index] = new_value;
	}
	else if (register_id <= Register::DI) {
        if ((registers[register_index] & 0xFFFF) != (new_value & 0xFFFF)) {
            register_change(register_id);
        }
		registers[register_index] |= new_value & 0xFFFF;
	}
	else if (register_id <= Register::BL) {
        if ((registers[register_index] & 0x00FF) != (new_value & 0x00FF)) {
            register_change(register_id);
        }
		registers[register_index] |= new_value & 0x00FF;
	}
	else if (register_id <= Register::BH) {
        if ((registers[register_index] & 0xFF00) != ((new_value & 0x00FF) << 8)) {
            register_change(register_id);
        }
		registers[register_index] |= (new_value & 0x00FF) << 8;
	}
	else if (register_id <= Register::GS) {
        if (segments[register_index] != new_value) {
            register_change(register_id);
        }
		segments[register_index] = new_value;
	}
	else if (register_id <= Register::CR1) {
		throw RegisterException("Control registers cannot be set directly", static_cast<U8>(register_id));
	}
	else {
		throw RegisterException("Wrong register id", static_cast<U8>(register_id));
	}
}


/**
 * Writes to a scalable register (EAX, EDX, etc...) from its ID
 */
void Registers::write(const Register register_id, const U32 new_value, OpSize size)
{
	U8 register_index = static_cast<U8>(register_id) & 0b111; // mod 8
    return write_index(register_index, new_value, size);
}


/**
 * Writes to a scalable register (EAX, EDX, etc...)
 */
void Registers::write_index(U8 register_index, U32 value, OpSize size)
{
    // TODO : separate those changes monitor from the other things
	switch (size)
	{
	case OpSize::B:
		if (register_index < 4) {
			// Low byte
            if ((registers[register_index] & 0x00FF) != (value & 0xFF)) {
                register_change(static_cast<Register>(register_index | 0b10000));
            }
			registers[register_index] |= value & 0xFF;
		}
		else {
			// High byte
            if ((registers[register_index] & 0xFF00) != ((value & 0xFF) << 8)) {
                register_change(static_cast<Register>(register_index | 0b10000));
            }
			registers[register_index] |= (value & 0xFF) << 8;
		}
		break;
	case OpSize::W:
        if ((registers[register_index] & 0xFFFF) != (value & 0xFFFF)) {
            register_change(static_cast<Register>(register_index | 0b01000));
        }
		registers[register_index] |= value & 0xFFFF;
		break;
	case OpSize::DW:
        if (registers[register_index] != value) {
            register_change(static_cast<Register>(register_index));
        }
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
    flags.reset();
	IDT_base = 0;
	IDT_limit = 0;
}


const char* Registers::register_to_string(Register reg)
{
	static std::map<Register, const char*> register_names{
		{ Register::EAX, "EAX" },
		{ Register::ECX, "ECX" },
		{ Register::EDX, "EDX" },
		{ Register::EBX, "EBX" },
		{ Register::ESP, "ESP" },
		{ Register::EBP, "EBP" },
		{ Register::ESI, "ESI" },
		{ Register::EDI, "EDI" },
		{ Register::AX,  "AX"  },
		{ Register::CX,  "CX"  },
		{ Register::DX,  "DX"  },
		{ Register::BX,  "BX"  },
		{ Register::SP,  "SP"  },
		{ Register::BP,  "BP"  },
		{ Register::SI,  "SI"  },
		{ Register::DI,  "DI"  },
		{ Register::AL,  "AL"  },
		{ Register::CL,  "CL"  },
		{ Register::DL,  "DL"  },
		{ Register::BL,  "BL"  },
		{ Register::AH,  "AH"  },
		{ Register::CH,  "CH"  },
		{ Register::DH,  "DH"  },
		{ Register::BH,  "BH"  },
		{ Register::CS,  "CS"  },
		{ Register::SS,  "SS"  },
		{ Register::DS,  "DS"  },
		{ Register::ES,  "ES"  },
		{ Register::FS,  "FS"  },
		{ Register::GS,  "GS"  },
		{ Register::CR0, "CR0" },
		{ Register::CR1, "CR1" }
	};

	auto it = register_names.find(reg);
	if (it == register_names.end()) {
		return nullptr;
	}
	else {
		return register_names[reg];
	}
}


std::string EFLAGS::print() const
{
    static const std::map<U32, const char*> flags_map{
        { CF, "CF" },
        { PF, "PF" },
        { AF, "AF" },
        { ZF, "ZF" },
        { SF, "SF" },
        { TF, "TF" },
        { IF, "IF" },
        { DF, "DF" },
        { OF, "OF" },
        { IOPL, "IOPL" },
        { IOPL_L, "IOPL_L" },
        { IOPL_H, "IOPL_H" },
        { NT, "NT" },
        { RF, "RF" },
        { VM, "VM" },
        { AC, "AC" },
        { VIF, "VIF" },
        { VIP, "VIP" },
        { ID, "ID" },
    };

    std::string str("[");

    for (const auto& [ flag, flag_str ] : flags_map ) {
        if (value & flag) {
            str += " ";
            str += flag_str;
        }
    }

    str += " ]";
    return str;
}
