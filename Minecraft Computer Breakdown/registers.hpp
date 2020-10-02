#pragma once

#include <memory>
#include <exception>

#include "data_types.h"

namespace Flags // todo: constexpr + replace everywhere for clarity
{
	const U32 CF = 1 << 0;
	const U32 PF = 1 << 2;
	const U32 AF = 1 << 4;
	const U32 ZF = 1 << 6;
	const U32 SF = 1 << 7;
	const U32 TF = 1 << 8;
	const U32 IF = 1 << 9;
	const U32 DF = 1 << 10;
	const U32 OF = 1 << 11;
	const U32 IOPL = 0b11 << 12;
	const U32 IOPL_L = 1 << 12;
	const U32 IOPL_H = 0b11 << 12;
	const U32 NT = 1 << 14;
	const U32 RF = 1 << 16;
	const U32 VM = 1 << 17;
	const U32 AC = 1 << 18;
	const U32 VIF = 1 << 19;
	const U32 VIP = 1 << 20;
	const U32 ID = 1 << 21;
};

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
		else if (register_id < 20) {
			return read_8_High(register_id);
		}
		else if (register_id < 24) {
			return read_8_Low(register_id);
		}
		else {
			throw std::logic_error("Wrong register id");
		}
	}
	
	template<typename N>
	N read_index(U8 register_index) const
	{
		switch (sizeof(N))
		{
		case sizeof(U8):
			if (register_index < 4) {
				// Low byte
				return registers[register_index] & 0xFF;
			} else {
				// High byte
				return (registers[register_index - 4] & 0xFF00) >> 8;
			}
		case sizeof(U16):
			return registers[register_index] & 0xFFFF;
		case sizeof(U32):
			return registers[register_index];
		default:
			throw std::logic_error("Wrong register size");
		}
	}
	
	void write(const U8 register_id, const U32 new_value, const U32 other_value = 0)
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
			throw std::logic_error("Wrong register id");
		}
	}

	template<typename N>
	void write_index(U8 register_index, U32 value)
	{
		switch (sizeof(N))
		{
		case sizeof(U8):
			if (register_index < 4) {
				// Low byte
				registers[register_index] |= value & 0xFF;
			}
			else {
				// High byte
				registers[register_index] |= (value & 0xFF) << 8;
			}
		case sizeof(U16):
			registers[register_index] |= value & 0xFFFF;
		case sizeof(U32):
			registers[register_index] = value;
		default:
			throw std::logic_error("Wrong register size");
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
		Segments Registers
	*/

	U16 segments[6] {
		// CS SS DS ES FS GS
		   0, 0, 0, 0, 0, 0
	};

	U16 segment_read_CS() const { return segments[0]; }
	U16 segment_read_SS() const { return segments[1]; }
	U16 segment_read_DS() const { return segments[2]; }
	U16 segment_read_ES() const { return segments[3]; }
	U16 segment_read_FS() const { return segments[4]; }
	U16 segment_read_GS() const { return segments[5]; }

	void segment_write_CS(U16 value) { segments[0] = value; }
	void segment_write_SS(U16 value) { segments[1] = value; }
	void segment_write_DS(U16 value) { segments[2] = value; }
	void segment_write_ES(U16 value) { segments[3] = value; }
	void segment_write_FS(U16 value) { segments[4] = value; }
	void segment_write_GS(U16 value) { segments[5] = value; }

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
	
	bit flag_read_CF() const { return bool(EFLAGS & (1 << 0)); }      // Carry Flag
	bit flag_read_PF() const { return bool(EFLAGS & (1 << 2)); }      // Parity Flag
	bit flag_read_AF() const { return bool(EFLAGS & (1 << 4)); }      // Adjust Flag
	bit flag_read_ZF() const { return bool(EFLAGS & (1 << 6)); }      // Zero Flag
	bit flag_read_SF() const { return bool(EFLAGS & (1 << 7)); }      // Sign Flag
	bit flag_read_TF() const { return bool(EFLAGS & (1 << 8)); }      // Trap Flag
	bit flag_read_IF() const { return bool(EFLAGS & (1 << 9)); }      // Interruption Flag
	bit flag_read_DF() const { return bool(EFLAGS & (1 << 10)); }     // Direction Flag
	bit flag_read_OF() const { return bool(EFLAGS & (1 << 11)); }     // Overflow Flag
	bit flag_read_IOPL_L() const { return bool(EFLAGS & (1 << 12)); } // first IOPL bit
	bit flag_read_IOPL_H() const { return bool(EFLAGS & (1 << 13)); } // second IOPL bit
	bit flag_read_NT() const { return bool(EFLAGS & (1 << 14)); }     // Nested Task Flag
	bit flag_read_RF() const { return bool(EFLAGS & (1 << 16)); }     // Resume Flag
	bit flag_read_VM() const { return bool(EFLAGS & (1 << 17)); }     // Virtual 8086 Mode Flag
	bit flag_read_AC() const { return bool(EFLAGS & (1 << 18)); }     // Alignment Check Flag
	bit flag_read_VIF() const { return bool(EFLAGS & (1 << 19)); }    // Virtual Interruption Flag
	bit flag_read_VIP() const { return bool(EFLAGS & (1 << 20)); }    // Virtual Interrupt Pending Flag
	bit flag_read_ID() const { return bool(EFLAGS & (1 << 21)); }     // Identification Flag

    void flag_write_CF(bit value) { EFLAGS |= value * (1 << 0); }     // Carry Flag
	void flag_write_PF(bit value) { EFLAGS |= value * (1 << 2); }	  // Parity Flag
	void flag_write_AF(bit value) { EFLAGS |= value * (1 << 4); }	  // Adjust Flag
	void flag_write_ZF(bit value) { EFLAGS |= value * (1 << 6); }	  // Zero Flag
	void flag_write_SF(bit value) { EFLAGS |= value * (1 << 7); }	  // Sign Flag
	void flag_write_TF(bit value) { EFLAGS |= value * (1 << 8); }	  // Trap Flag
	void flag_write_IF(bit value) { EFLAGS |= value * (1 << 9); }	  // Interruption Flag
	void flag_write_DF(bit value) { EFLAGS |= value * (1 << 10); }	  // Direction Flag
	void flag_write_OF(bit value) { EFLAGS |= value * (1 << 11); }	  // Overflow Flag
	void flag_write_IOPL_L(bit value) { EFLAGS |= value * (1 << 12); }// first IOPL bit
	void flag_write_IOPL_H(bit value) { EFLAGS |= value * (1 << 13); }// second IOPL bit
	void flag_write_NT(bit value) { EFLAGS |= value * (1 << 14); }	  // Nested Task Flag
	void flag_write_RF(bit value) { EFLAGS |= value * (1 << 16); }    // Resume Flag
	void flag_write_VM(bit value) { EFLAGS |= value * (1 << 17); }    // Virtual 8086 Mode Flag
	void flag_write_AC(bit value) { EFLAGS |= value * (1 << 18); }    // Alignment Check Flag
	void flag_write_VIF(bit value) { EFLAGS |= value * (1 << 19); }   // Virtual Interruption Flag
	void flag_write_VIP(bit value) { EFLAGS |= value * (1 << 20); }   // Virtual Interrupt Pending Flag
	void flag_write_ID(bit value) { EFLAGS |= value * (1 << 21); }    // Identification Flag

	/*
		Control Registers
	*/
	U32 control_registers[4] {
		// CR0 CR1 CR2 CR3
			0,  0,  0,  0
	};

	U32 read_control_register(U8 index) const { return control_registers[index]; }
	void write_control_register(U8 index, U32 value) { control_registers[index] = value; }

	bit control_flag_read_PE() const { return bool(control_registers[0] & (1 << 0)); }  // Protection Enable Flag
	bit control_flag_read_MP() const { return bool(control_registers[0] & (1 << 1)); }  // Math Present Flag
	bit control_flag_read_EM() const { return bool(control_registers[0] & (1 << 2)); }  // Emulation Flag
	bit control_flag_read_TS() const { return bool(control_registers[0] & (1 << 3)); }  // Task Switched Flag
	bit control_flag_read_ET() const { return bool(control_registers[0] & (1 << 4)); }  // Extension Type Flag
	bit control_flag_read_PG() const { return bool(control_registers[0] & (1 << 31)); } // Paging Flag
	
	void control_flag_write_PE(bit value) { control_registers[0] |= value * (1 << 0); }  // Protection Enable Flag
	void control_flag_write_MP(bit value) { control_registers[0] |= value * (1 << 1); }  // Math Present Flag
	void control_flag_write_EM(bit value) { control_registers[0] |= value * (1 << 2); }  // Emulation Flag
	void control_flag_write_TS(bit value) { control_registers[0] |= value * (1 << 3); }  // Task Switched Flag
	void control_flag_write_ET(bit value) { control_registers[0] |= value * (1 << 4); }  // Extension Type Flag
	void control_flag_write_PG(bit value) { control_registers[0] |= value * (1 << 31); } // Paging Flag

	/*
		Utilities
	*/

	void reset_general_purpose_registers()
	{
		std::memset(registers, 0, 8);
	}

	void reset_segments_registers()
	{
		std::memset(segments, 0, 6);
	}

	void reset_control_registers()
	{
		std::memset(control_registers, 0, 4);
	}

	void complete_reset()
	{
		reset_general_purpose_registers();
		reset_segments_registers();
		reset_control_registers();

		EIP = 0;
		EFLAGS = 0b10;
	}
};
