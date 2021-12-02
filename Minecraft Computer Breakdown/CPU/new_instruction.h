#pragma once

#include "../data_types.h"


enum class Register : U8 {
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


/**
 * Instruction for our computer.
 *
 * It is not encoded, which simplifies the circuitry by a lot, however this comes at the cost of bigger
 * memory usage, but by design this is not a problem. The only downside is that the executable files are
 * bigger.
 *
 * Total size: 112 bits used, but 128 bits in memory
 */
struct Instruction {

    U8 opcode;

    // TODO : maybe store some of the bits required for computing the address in the register bits of the operand which
    //  has the type MEM, like reg and scale (scale is NOT the size of the register!) -> yeah do that yeah good idea

    /// Struct describing an operand, present in order for IA32::Mapping::convert_operand to exist.
    /// An additional field for 'write' would have been great but the struct is already 8 bits long.
    struct Operand {
        OpType type : 2;
        Register reg : 5;
        bit read : 1;

        // TODO : re-enable constexpr when the GCC bug is fixed
        //  See: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=102490#c0
        /*constexpr*/ bit operator==(const Operand& other) const = default;
    } op1, op2;

    // TODO : docs for all fields

    // Flags
    bit operand_size_override : 1;
    bit operand_byte_size_override : 1;

    // No address size override, since we impose 32-bit addressing for simplicity

    bit get_flags : 1;
    bit get_CR0 : 1;

    // Output
    bit write_ret1_to_op1 : 1;
    bit write_ret2_to_op2 : 1;

    bit write_ret2_to_register : 1;
    bit scale_output_override : 1;
    Register register_out : 5;

    // Addressing
    // An effective address needs to be computed if any of reg_present, base_present or displacement_present is true
    // Formula for the address: [reg*(scale) + base] + disp, if any is not present, it is replaced by zero
    bit reg_present : 1;   // If 'reg' contains a register index
    U8 reg : 3;            // Register used as an index to the memory
    U8 scale : 2;          // Scale of the register index ('0b00' -> 1, '0b01' -> 2, '0b10' -> 4, '0b11' -> 8)
    bit base_present : 1;  // If 'base_reg' contains a register index
    U8 base_reg : 3;       // Register used as a base to the memory
    U8 displacement_present : 1; // The displacement is stored in the address_value field

    U8 : 0; // alignment (16 bits)

    // both of those values can be used as general purpose values in special instructions (bound, call...)
    U32 address_value;
    U32 immediate_value;

    // The Operand struct by itself cannot describe when it is not used, here is how to do it for both operands:
    [[nodiscard]] constexpr bool is_op1_none() const { return !op1.read && !write_ret1_to_op1; }
    [[nodiscard]] constexpr bool is_op2_none() const { return !op2.read && !write_ret2_to_op2; }

    /*constexpr*/ bool operator==(const Instruction& other) const = default; // TODO : re-enable constexpr when the GCC bug is fixed
};
