#pragma once

#include <memory>
#include <exception>

#include "data_types.h"


struct Registers
{
	/*
		General-Purpose registers
	*/

	U32 registers[8] {
		// EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI
		     0,   0,   0,   0,   0,   0,   0,   0
	};

	U32 read(const U8 register_id) const 
	{
		if (register_id < 8) {
			return read_32(register_id);
		}
		else if (register_id < 16) {
			return read_16(register_id);
		}
		else if (register_id < 24) {
			return read_8_High(register_id);
		}
		else if (register_id < 32) {
			return read_8_Low(register_id);
		}
		else {
			throw std::logic_error("Wrong register id");
		}
	}

	void write(const U8 register_id, const U32 new_value)
	{
		if (register_id < 8) {
			write_32(register_id, new_value);
		}
		else if (register_id < 16) {
			write_16(register_id, new_value);
		}
		else if (register_id < 24) {
			write_8_High(register_id, new_value);
		}
		else if (register_id < 32) {
			write_8_Low(register_id, new_value);
		}
		else {
			throw std::logic_error("Wrong register id");
		}
	}

	U32 read_32(const U8 register_id) const     { return registers[register_id % 8]; }
	U16 read_16(const U8 register_id) const     { return registers[register_id % 8] & 0xFFFF; }
	U16 read_8_High(const U8 register_id) const { return (registers[register_id % 8] & 0xFF00) >> 8; }
	U16 read_8_Low(const U8 register_id) const  { return registers[register_id % 8] & 0xFF; }

	void write_32(const U8 register_id, U32 new_value)    { registers[register_id % 8] = new_value; }
	void write_16(const U8 register_id, U16 new_value)    { registers[register_id % 8] = (registers[register_id % 8] & 0xFFFF0000) | new_value; }
	void write_8_High(const U8 register_id, U8 new_value) { registers[register_id % 8] = (registers[register_id % 8] & 0xFFFF00FF) | (new_value << 8); }
	void write_8_Low(const U8 register_id, U8 new_value)  { registers[register_id % 8] = (registers[register_id % 8] & 0xFFFFFF00) | new_value; }

	/*
		Instruction pointer
	*/

	U32 EIP = 0;
	U32 read_EIP() const { return EIP; }
	void write_EIP(U32 value) { EIP = value; }

	/*
		Flags
	*/

	U32 EFLAGS = 0b10; // bit 1 is always 1

	U32 read_EFLAGS() const { return EFLAGS; }
	U16 read_FLAGS() const { return EFLAGS & 0xFFFF; }

	bit flag_read_CF() const { return bool(EFLAGS & 0x001); }      // Carry Flag
	bit flag_read_PF() const { return bool(EFLAGS & 0x004); }      // Parity Flag
	bit flag_read_AF() const { return bool(EFLAGS & 0x010); }      // Adjust Flag
	bit flag_read_ZF() const { return bool(EFLAGS & 0x040); }      // Zero Flag
	bit flag_read_SF() const { return bool(EFLAGS & 0x080); }      // Sign Flag
	bit flag_read_TF() const { return bool(EFLAGS & 0x100); }      // Trap Flag
	bit flag_read_IF() const { return bool(EFLAGS & 0x200); }      // Interruption Flag
	bit flag_read_DF() const { return bool(EFLAGS & 0x400); }      // Direction Flag
	bit flag_read_OF() const { return bool(EFLAGS & 0x800); }      // Overflow Flag
	bit flag_read_IOPL_L() const { return bool(EFLAGS & 0x1000); } // first IOPL bit
	bit flag_read_IOPL_H() const { return bool(EFLAGS & 0x2000); } // second IOPL bit
	bit flag_read_NT() const { return bool(EFLAGS & 0x4000); }     // Nested Task Flag
	bit flag_read_RF() const { return bool(EFLAGS & 0x10000); }   // Resume Flag
	bit flag_read_VM() const { return bool(EFLAGS & 0x20000); }   // Virtual 8086 Mode Flag
	bit flag_read_AC() const { return bool(EFLAGS & 0x40000); }   // Alignment Check Flag
	bit flag_read_VIF() const { return bool(EFLAGS & 0x80000); }  // Virtual Interruption Flag
	bit flag_read_VIP() const { return bool(EFLAGS & 0x100000); } // Virtual Interrupt Pending Flag
	bit flag_read_ID() const { return bool(EFLAGS & 0x200000); }  // Identification Flag

    void flag_write_CF(bit value) { EFLAGS |= value * 0x001; }     // Carry Flag
	void flag_write_PF(bit value) { EFLAGS |= value * 0x004; }	   // Parity Flag
	void flag_write_AF(bit value) { EFLAGS |= value * 0x010; }	   // Adjust Flag
	void flag_write_ZF(bit value) { EFLAGS |= value * 0x040; }	   // Zero Flag
	void flag_write_SF(bit value) { EFLAGS |= value * 0x080; }	   // Sign Flag
	void flag_write_TF(bit value) { EFLAGS |= value * 0x100; }	   // Trap Flag
	void flag_write_IF(bit value) { EFLAGS |= value * 0x200; }	   // Interruption Flag
	void flag_write_DF(bit value) { EFLAGS |= value * 0x400; }	   // Direction Flag
	void flag_write_OF(bit value) { EFLAGS |= value * 0x800; }	   // Overflow Flag
	void flag_write_IOPL_L(bit value) { EFLAGS |= value * 0x1000; }// first IOPL bit
	void flag_write_IOPL_H(bit value) { EFLAGS |= value * 0x2000; }// second IOPL bit
	void flag_write_NT(bit value) { EFLAGS |= value * 0x4000; }	   // Nested Task Flag
	void flag_write_RF(bit value) { EFLAGS |= value * 0x10000; }  // Resume Flag
	void flag_write_VM(bit value) { EFLAGS |= value * 0x20000; }  // Virtual 8086 Mode Flag
	void flag_write_AC(bit value) { EFLAGS |= value * 0x40000; }  // Alignment Check Flag
	void flag_write_VIF(bit value) { EFLAGS |= value * 0x80000; } // Virtual Interruption Flag
	void flag_write_VIP(bit value) { EFLAGS |= value * 0x100000; }// Virtual Interrupt Pending Flag
	void flag_write_ID(bit value) { EFLAGS |= value * 0x200000; } // Identification Flag

	/*
		Utilities
	*/

	void reset_GPRs()
	{
		std::memset(registers, 0, 8);
	}

	void complete_reset()
	{
		reset_GPRs();

		EIP = 0;
		EFLAGS = 0b10;
	}
};
