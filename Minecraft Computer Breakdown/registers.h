#pragma once

#include <memory>

#include "data_types.h"


struct Registers
{
	/*
		General-Purpose registers
	*/

	U32 registers[8]{
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
			return 0;
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
	}

	U32 read_32(const U8 register_id) const     { return registers[register_id % 8]; }
	U16 read_16(const U8 register_id) const     { return registers[register_id % 8] & 0xFF'FF; }
	U16 read_8_High(const U8 register_id) const { return (registers[register_id % 8] & 0xFF'00) >> 8; }
	U16 read_8_Low(const U8 register_id) const  { return registers[register_id % 8] & 0xFF; }

	void write_32(const U8 register_id, U32 new_value)    { registers[register_id % 8] = new_value; }
	void write_16(const U8 register_id, U16 new_value)    { registers[register_id % 8] = (registers[register_id % 8] & 0xFF'FF'00'00) | new_value; }
	void write_8_High(const U8 register_id, U8 new_value) { registers[register_id % 8] = (registers[register_id % 8] & 0xFF'FF'00'FF) | (new_value << 8); }
	void write_8_Low(const U8 register_id, U8 new_value)  { registers[register_id % 8] = (registers[register_id % 8] & 0xFF'FF'FF'00) | new_value; }

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
	U16 read_FLAGS() const { return EFLAGS & 0xFF'FF; }

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
	bit flag_read_RF() const { return bool(EFLAGS & 0x1'0000); }   // Resume Flag
	bit flag_read_VM() const { return bool(EFLAGS & 0x2'0000); }   // Virtual 8086 Mode Flag
	bit flag_read_AC() const { return bool(EFLAGS & 0x4'0000); }   // Alignment Check Flag
	bit flag_read_VIF() const { return bool(EFLAGS & 0x8'0000); }  // Virtual Interruption Flag
	bit flag_read_VIP() const { return bool(EFLAGS & 0x10'0000); } // Virtual Interrupt Pending Flag
	bit flag_read_ID() const { return bool(EFLAGS & 0x20'0000); }  // Identification Flag

	/* Carry Flag */ void flag_write_CF(bit value) { EFLAGS |= value * 0x001; }     // 
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
	void flag_write_RF(bit value) { EFLAGS |= value * 0x1'0000; }  // Resume Flag
	void flag_write_VM(bit value) { EFLAGS |= value * 0x2'0000; }  // Virtual 8086 Mode Flag
	void flag_write_AC(bit value) { EFLAGS |= value * 0x4'0000; }  // Alignment Check Flag
	void flag_write_VIF(bit value) { EFLAGS |= value * 0x8'0000; } // Virtual Interruption Flag
	void flag_write_VIP(bit value) { EFLAGS |= value * 0x10'0000; }// Virtual Interrupt Pending Flag
	void flag_write_ID(bit value) { EFLAGS |= value * 0x20'0000; } // Identification Flag

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




/*
	Container for reading / writing all x86 registers
*/
struct Registers__
{
	/*
		General-Purpose registers
	*/

	U32 EAX = 0,
		ECX = 0, 
		EDX = 0, 
		EBX = 0,
		ESP = 0, 
		EBP = 0,
		ESI = 0, 
		EDI = 0;

	U32 read_EAX() const { return EAX; }
	U32 read_ECX() const { return ECX; }
	U32 read_EDX() const { return EDX; }
	U32 read_EBX() const { return EBX; }
	U32 read_ESP() const { return ESP; }
	U32 read_EBP() const { return EBP; }
	U32 read_ESI() const { return ESI; }
	U32 read_EDI() const { return EDI; }

	void write_EAX(U32 value) { EAX = value; }
	void write_ECX(U32 value) { ECX = value; }
	void write_EDX(U32 value) { EDX = value; }
	void write_EBX(U32 value) { EBX = value; }
	void write_ESP(U32 value) { ESP = value; }
	void write_EBP(U32 value) { EBP = value; }
	void write_ESI(U32 value) { ESI = value; }
	void write_EDI(U32 value) { EDI = value; }
		
	U16 read_AX() const { return EAX & 0xFF'FF; }
	U16 read_CX() const { return ECX & 0xFF'FF; }
	U16 read_DX() const { return EDX & 0xFF'FF; }
	U16 read_BX() const { return EBX & 0xFF'FF; }
	U16 read_SP() const { return ESP & 0xFF'FF; }
	U16 read_BP() const { return EBP & 0xFF'FF; }
	U16 read_SI() const { return ESI & 0xFF'FF; }
	U16 read_DI() const { return EDI & 0xFF'FF; }
	
	void write_AX(U16 value) { EAX = value; }
	void write_CX(U16 value) { ECX = value; }
	void write_DX(U16 value) { EDX = value; }
	void write_BX(U16 value) { EBX = value; }
	void write_SP(U16 value) { ESP = value; }
	void write_BP(U16 value) { EBP = value; }
	void write_SI(U16 value) { ESI = value; }
	void write_DI(U16 value) { EDI = value; }
	
	U8 read_AH() const { return EAX & 0xFF'00; }
	U8 read_CH() const { return ECX & 0xFF'00; }
	U8 read_DH() const { return EDX & 0xFF'00; }
	U8 read_BH() const { return EBX & 0xFF'00; }
	U8 read_SPH() const { return ESP & 0xFF'00; }
	U8 read_BPH() const { return EBP & 0xFF'00; }
	U8 read_SIH() const { return ESI & 0xFF'00; }
	U8 read_DIH() const { return EDI & 0xFF'00; }

	void write_AH(U8 value) { EAX = value << 8; }
	void write_CH(U8 value) { ECX = value << 8; }
	void write_DH(U8 value) { EDX = value << 8; }
	void write_BH(U8 value) { EBX = value << 8; }
	void write_SPH(U8 value) { ESP = value << 8; }
	void write_BPH(U8 value) { EBP = value << 8; }
	void write_SIH(U8 value) { ESI = value << 8; }
	void write_DIH(U8 value) { EDI = value << 8; }

	U8 read_AL() const { return EAX & 0xFF; }
	U8 read_CL() const { return ECX & 0xFF; }
	U8 read_DL() const { return EDX & 0xFF; }
	U8 read_BL() const { return EBX & 0xFF; }
	U8 read_SPL() const { return ESP & 0xFF; }
	U8 read_BPL() const { return EBP & 0xFF; }
	U8 read_SIL() const { return ESI & 0xFF; }
	U8 read_DIL() const { return EDI & 0xFF; }

	void write_AL(U8 value) { EAX = value; }
	void write_CL(U8 value) { ECX = value; }
	void write_DL(U8 value) { EDX = value; }
	void write_BL(U8 value) { EBX = value; }
	void write_SPL(U8 value) { ESP = value; }
	void write_BPL(U8 value) { EBP = value; }
	void write_SIL(U8 value) { ESI = value; }
	void write_DIL(U8 value) { EDI = value; }

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
	U16 read_FLAGS() const { return EFLAGS & 0xFF'FF; }

	bit flag_read_CF() const { return bool(EFLAGS & 0x001); } // Carry Flag
	bit flag_read_PF() const { return bool(EFLAGS & 0x004); } // Parity Flag
	bit flag_read_AF() const { return bool(EFLAGS & 0x010); } // Adjust Flag
	bit flag_read_ZF() const { return bool(EFLAGS & 0x040); } // Zero Flag
	bit flag_read_SF() const { return bool(EFLAGS & 0x080); } // Sign Flag
	bit flag_read_TF() const { return bool(EFLAGS & 0x100); } // Trap Flag
	bit flag_read_IF() const { return bool(EFLAGS & 0x200); } // Interruption Flag
	bit flag_read_DF() const { return bool(EFLAGS & 0x400); } // Direction Flag
	bit flag_read_OF() const { return bool(EFLAGS & 0x800); } // Overflow Flag
	bit flag_read_IOPL_L() const { return bool(EFLAGS & 0x1000); } // first IOPL bit
	bit flag_read_IOPL_H() const { return bool(EFLAGS & 0x2000); } // second IOPL bit
	bit flag_read_NT() const { return bool(EFLAGS & 0x4000); } // Nested Task Flag
	bit flag_read_RF() const { return bool(EFLAGS & 0x1'0000); } // Resume Flag
	bit flag_read_VM() const { return bool(EFLAGS & 0x2'0000); } // Virtual 8086 Mode Flag
	bit flag_read_AC() const { return bool(EFLAGS & 0x4'0000); } // Alignment Check Flag
	bit flag_read_VIF() const { return bool(EFLAGS & 0x8'0000); } // Virtual Interruption Flag
	bit flag_read_VIP() const { return bool(EFLAGS & 0x10'0000); } // Virtual Interrupt Pending Flag
	bit flag_read_ID() const { return bool(EFLAGS & 0x20'0000); } // Identification Flag

	void flag_write_CF(bit value) { EFLAGS |= value * 0x001; }
	void flag_write_PF(bit value) { EFLAGS |= value * 0x004; }
	void flag_write_AF(bit value) { EFLAGS |= value * 0x010; }
	void flag_write_ZF(bit value) { EFLAGS |= value * 0x040; }
	void flag_write_SF(bit value) { EFLAGS |= value * 0x080; }
	void flag_write_TF(bit value) { EFLAGS |= value * 0x100; }
	void flag_write_IF(bit value) { EFLAGS |= value * 0x200; }
	void flag_write_DF(bit value) { EFLAGS |= value * 0x400; }
	void flag_write_OF(bit value) { EFLAGS |= value * 0x800; }
	void flag_write_IOPL_L(bit value) { EFLAGS |= value * 0x1000; }
	void flag_write_IOPL_H(bit value) { EFLAGS |= value * 0x2000; }
	void flag_write_NT(bit value) { EFLAGS |= value * 0x4000; }
	void flag_write_RF(bit value) { EFLAGS |= value * 0x1'0000; }
	void flag_write_VM(bit value) { EFLAGS |= value * 0x2'0000; }
	void flag_write_AC(bit value) { EFLAGS |= value * 0x4'0000; }
	void flag_write_VIF(bit value) { EFLAGS |= value * 0x8'0000; }
	void flag_write_VIP(bit value) { EFLAGS |= value * 0x10'0000; }
	void flag_write_ID(bit value) { EFLAGS |= value * 0x20'0000; }

	/*
		Utilities
	*/

	void reset_GPRs()
	{
		EAX = 0;
		ECX = 0;
		EDX = 0;
		EBX = 0;
		ESP = 0;
		EBP = 0;
		ESI = 0;
		EDI = 0;
	}

	void complete_reset()
	{
		reset_GPRs();

		EIP = 0;
		EFLAGS = 0b10;
	}
};