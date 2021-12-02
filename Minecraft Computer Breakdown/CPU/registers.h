#pragma once

#include <memory>

#include "../data_types.h"
#include "../ALU.hpp"
#include "instructions.h"


namespace Register
{
	// arbitrary values for each register

	const U8 EAX = 0;
	const U8 ECX = 1;
	const U8 EDX = 2;
	const U8 EBX = 3;
	const U8 ESP = 4;
	const U8 EBP = 5;
	const U8 ESI = 6;
	const U8 EDI = 7;

	const U8 AX = 8;
	const U8 CX = 9;
	const U8 DX = 10;
	const U8 BX = 11;
	const U8 SP = 12;
	const U8 BP = 13;
	const U8 SI = 14;
	const U8 DI = 15;

	const U8 AH = 16;
	const U8 CH = 17;
	const U8 DH = 18;
	const U8 BH = 19;

	const U8 AL = 20;
	const U8 CL = 21;
	const U8 DL = 22;
	const U8 BL = 23;

	const U8 CS = 0;
	const U8 SS = 1;
	const U8 DS = 2;
	const U8 ES = 3;
	const U8 FS = 4;
	const U8 GS = 5;
	
	const U8 CR0 = 0;
	const U8 CR1 = 1;
	const U8 CR3 = 2;
	
	// TODO : replace by:
/*
enum class Register : uint8_t {
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
    CR1 = 0b11000 | EDI,
};
*/
};


template<typename Value_t>
class FlagsInterface
{
public:
    explicit FlagsInterface(Value_t value) : value(value) { }

    virtual void reset() { }

    [[nodiscard]] constexpr bit get(Value_t flag) const
    { return bool(value & Value_t(flag)); }

    constexpr void set(Value_t flag)
    { value |= Value_t(flag); }

    constexpr void clear(Value_t flag)
    { value &= ~Value_t(flag); }

    constexpr void set_val(Value_t flag, bit val)
    {
        value &= ~Value_t(flag);    // ignore the previous value of the flag
        if (val) {
            value |= Value_t(flag); // and set it to the new value
        }
    }

    Value_t value;
};


class EFLAGS : public FlagsInterface<U32>
{
public:
    static constexpr U32 CF	    = 1 << 0;
    static constexpr U32 PF	    = 1 << 2;
    static constexpr U32 AF	    = 1 << 4;
    static constexpr U32 ZF	    = 1 << 6;
    static constexpr U32 SF	    = 1 << 7;
    static constexpr U32 TF	    = 1 << 8;
    static constexpr U32 IF	    = 1 << 9;
    static constexpr U32 DF	    = 1 << 10;
    static constexpr U32 OF	    = 1 << 11;
    static constexpr U32 IOPL	= 0b11 << 12;
    static constexpr U32 IOPL_L = 1 << 12;
    static constexpr U32 IOPL_H = 0b10 << 12;
    static constexpr U32 NT	    = 1 << 14;
    static constexpr U32 RF	    = 1 << 16;
    static constexpr U32 VM	    = 1 << 17;
    static constexpr U32 AC	    = 1 << 18;
    static constexpr U32 VIF	= 1 << 19;
    static constexpr U32 VIP	= 1 << 20;
    static constexpr U32 ID	    = 1 << 21;

    static constexpr U32 default_value = 0b10; // bit 1 is always 1

    explicit EFLAGS(U32 value = default_value) : FlagsInterface(value) { }

    void reset() override
    { value = default_value; }

    /*
     * The following methods implements the most used logic for updating a flag after an instruction.
     */

    constexpr void update_sign_flag(U32 result, OpSize size)
    { set_val(EFLAGS::SF, ALU::check_is_negative(result, size)); }

    constexpr void update_zero_flag(U32 result)
    { set_val(EFLAGS::ZF, ALU::check_equal_zero(result)); }

    constexpr void update_parity_flag(U32 result)
    {
        // Parity check is made only on the first byte
        set_val(EFLAGS::PF, ALU::check_parity(U8(result & 0xFF)));
    }

    constexpr void update_overflow_flag(U32 op1, U32 op2, U32 result, OpSize op1Size, OpSize op2Size, OpSize retSize)
    {
        /*
        overflow flag truth table (1: op1, 2: op2, R: result):
        1 2 R OF
        + + + 0
        + + - 1
        + - / 0
        - + / 0
        - - - 0
        - - + 1

        => (sign(op1) & sign(op2)) ^ sign(R)
        */
        bit is_op1_neg = ALU::check_is_negative(op1, op1Size);
        bit is_op2_neg = ALU::check_is_negative(op2, op2Size);
        bit is_ret_neg = ALU::check_is_negative(result, retSize);
        bit val = (is_op1_neg & is_op2_neg) ^ is_ret_neg;
        set_val(EFLAGS::OF, val);
    }

    [[nodiscard]] constexpr U8 read_IOPL() const { return (value & (EFLAGS::IOPL)) >> 12; }
};


class CR0 : public FlagsInterface<U32>
{
public:

    static constexpr U32 PE = 1 << 0;  // Protection Enable Flag
    static constexpr U32 MP = 1 << 1;  // Math Present Flag
    static constexpr U32 EM = 1 << 2;  // Emulation Flag
    static constexpr U32 TS = 1 << 3;  // Task Switched Flag
    static constexpr U32 ET = 1 << 4;  // Extension Type Flag
    static constexpr U32 PG = 1 << 31; // Paging Flag

    explicit CR0(U32 value = 0) : FlagsInterface<U32>(value) { }

    void reset() override
    { value = 0; }
};


struct Registers
{
	/**
	 * General-Purpose registers
	 */
	U32 registers[8] {
		// EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI
	        0,   0,   0,   0,   0,   0,   0,   0
	};

	[[nodiscard]] U32 read(const U8 register_id) const;
	[[nodiscard]] U32 read_index(U8 register_index, OpSize size) const;
	
	void write(const U8 register_id, const U32 new_value, const U32 other_value = 0);
	void write_index(U8 register_index, U32 value, OpSize size);

	[[nodiscard]] U32 read_32(const U8 register_id) const     { return registers[register_id % 8]; }
	[[nodiscard]] U16 read_16(const U8 register_id) const     { return registers[register_id % 8] & 0xFFFF; }
	[[nodiscard]] U16 read_8_High(const U8 register_id) const { return (registers[register_id % 8] & 0xFF00) >> 8; }
	[[nodiscard]] U16 read_8_Low(const U8 register_id) const  { return registers[register_id % 8] & 0xFF; }

	void write_32(const U8 register_id, U32 new_value)    { registers[register_id % 8] = new_value; }
	void write_16(const U8 register_id, U16 new_value)    { registers[register_id % 8] = (registers[register_id % 8] & 0xFFFF0000) | new_value; }
	void write_8_High(const U8 register_id, U8 new_value) { registers[register_id % 8] = (registers[register_id % 8] & 0xFFFF00FF) | (new_value << 8); }
	void write_8_Low(const U8 register_id, U8 new_value)  { registers[register_id % 8] = (registers[register_id % 8] & 0xFFFFFF00) | new_value; }


	/**
	 * Segments Registers
	 */
	U16 segments[6] {
		// CS SS DS ES FS GS
		   0, 0, 0, 0, 0, 0
	};
	
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
