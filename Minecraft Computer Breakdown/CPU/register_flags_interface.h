#pragma once

#include "../data_types.h"
#include "../ALU.hpp"


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
