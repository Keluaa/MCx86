#pragma once

#include <memory>

#include "../data_types.h"
#include "../ALU.hpp"
#include "register_flags_interface.h"


/**
 * Register indexes. All indexes fit on 5 bits.
 */
enum class Register : U8
{
    EAX = 0,
    ECX = 1,
    EDX = 2,
    EBX = 3,
    ESP = 4,
    EBP = 5,
    ESI = 6,
    EDI = 7,

    AX = 0b01000 | EAX,
    CX = 0b01000 | ECX,
    DX = 0b01000 | EDX,
    BX = 0b01000 | EBX,
    SP = 0b01000 | ESP,
    BP = 0b01000 | EBP,
    SI = 0b01000 | ESI,
    DI = 0b01000 | EDI,

    AL = 0b10000 | EAX,
    CL = 0b10000 | ECX,
    DL = 0b10000 | EDX,
    BL = 0b10000 | EBX,
    AH = 0b10000 | ESP,
    CH = 0b10000 | EBP,
    DH = 0b10000 | ESI,
    BH = 0b10000 | EDI,

    CS = 0b11000 | EAX,
    SS = 0b11000 | ECX,
    DS = 0b11000 | EDX,
    ES = 0b11000 | EBX,
    FS = 0b11000 | ESP,
    GS = 0b11000 | EBP,

    CR0 = 0b11000 | ESI,
    CR1 = 0b11000 | EDI
};


struct Registers
{
    // TODO : branch multi-access checking

	/**
	 * General-Purpose registers
	 */
	U32 registers[8] {
		// EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI
	        0,   0,   0,   0,   0,   0,   0,   0
	};

	[[nodiscard]] U32 read(const Register register_id) const;
	[[nodiscard]] U32 read_index(U8 register_index, OpSize size) const;
	
	void write(const Register register_val, const U32 new_value);
	void write_index(U8 register_index, U32 value, OpSize size);


	/**
	 * Segments Registers
	 */
	U16 segments[6] {
		// CS SS DS ES FS GS
		   0, 0, 0, 0, 0, 0
	};
	
	[[nodiscard]] U16 read_segment(Register register_id) const { return segments[static_cast<U8>(register_id) % 8]; }
	[[nodiscard]] U16 read_segment(U8 index) const { return segments[index]; }
	void write_segment(U8 index, U16 value) { segments[index] = value; }


	/**
	 * Instruction pointer
	 */
	U32 EIP = 0;
	[[nodiscard]] U32 read_EIP() const { return EIP; }
	void write_EIP(U32 value) { EIP = value; }


	/**
	 * Status Flags
	 */
	U32 EFLAGS = 0b10; // bit 1 is always 1


	/**
	 * Control Registers
	 */
	U32 control_registers[4] {
		// CR0 CR1 CR2 CR3
			0,  0,  0,  0
	};

	[[nodiscard]] CR0 get_CR0() const { return CR0(control_registers[0]); }
	void set_CR0(const CR0& cr0) { control_registers[0] = cr0.value; }

	[[nodiscard]] U32 read_control_register(U8 index) const { return control_registers[index]; }
	void write_control_register(U8 index, U32 value) { control_registers[index] = value; }


	/**
	 * Interrupt Descriptor Table Register (IDTR)
	 */
	U32 IDT_base = 0;
	U16 IDT_limit = 0;

	[[nodiscard]] U32 read_IDT_base() const { return IDT_base; }
	[[nodiscard]] U16 read_IDT_limit() const { return IDT_limit; }
	void write_IDTR(U32 IDT_address, U8 size) { IDT_base = IDT_address; IDT_limit = size << 3; }


	/**
	 * Utilities
	 */
	void reset_general_purpose_registers();
	void reset_segments_registers();
	void reset_control_registers();
	void complete_reset();
};
