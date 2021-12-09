#pragma once

#include "../data_types.h"
#include "registers.h"


// TODO : docs + rework a bit
struct InstData
{
	U32 op1, op2, op3;
	
	U32 address;
	U32 imm;
	
	OpSize op1_size : 2;
	OpSize op2_size : 2;
	OpSize op3_size : 2;

	// Used only for non-arithmetic instructions
	OpSize op_size : 2;
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
struct Inst
{
	U8 opcode;

    // TODO : maybe store some of the bits required for computing the address in the register bits of the operand which
    //  has the type MEM, like reg and scale (scale is NOT the size of the register!) -> yeah do that yeah good idea

    /**
     * Struct describing an operand.
     * An additional field for 'write' would have been great but the struct is already 8 bits long.
     */
    struct Operand {
        OpType type : 2;  // Type of the operand
        Register reg : 5; // The register of the operand (if any)
        bit read : 1;     // Load the value of the operand before executing the instruction

        // TODO : re-enable constexpr when the GCC bug is fixed
        //  See: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=102490#c0
        /*constexpr*/ bit operator==(const Operand& other) const = default;
    } op1, op2;

    // Flags
    bit operand_size_override : 1;      // Operand size override (0->32, 1->16)
    bit operand_byte_size_override : 1; // Operand byte size override

    // No address size override, since we impose 32-bit addressing for simplicity

    bit get_flags : 1; // Access flags, then write them back after the instruction execution
    bit get_CR0 : 1;   // TODO : implement

    // Output
	bit write_ret1_to_op1 : 1; // Write the first result to the location of the first operand
	bit write_ret2_to_op2 : 1; // Write the second result to the location of the second operand

	bit write_ret2_to_register : 1; // Write the second return value to a specific register
	bit scale_output_override : 1;  // Scale the specific register to match the size of the operand
	Register register_out : 5;      // The specific register

    // Addressing
    // An effective address needs to be computed if any of reg_present, base_present or displacement_present is true
    // Formula for the address: [reg*(scale) + base] + disp, if any is not present, it is replaced by zero
    bit reg_present : 1;   // If 'reg' contains a register index
    U8 reg : 3;            // Exx register used as an index to the memory
    U8 scale : 2;          // Scale of the register index ('0b00' -> 1, '0b01' -> 2, '0b10' -> 4, '0b11' -> 8)
    bit base_present : 1;  // If 'base_reg' contains a register index
    U8 base_reg : 3;       // Register used as a base to the memory
    U8 displacement_present : 1; // The displacement is stored in the address_value field
	
	U8 : 0; // alignment

	// both of those values can be used as general purpose values in spacial instructions (bound, call...)
	U32 address_value;
	U32 immediate_value;

    Inst() = default;

    // The Operand struct by itself cannot describe when it is not used, here is how to do it for both operands:
    [[nodiscard]] constexpr bool is_op1_none() const { return !op1.read && !write_ret1_to_op1; }
    [[nodiscard]] constexpr bool is_op2_none() const { return !op2.read && !write_ret2_to_op2; }

    [[nodiscard]] constexpr U8 op1_reg_index() const { return static_cast<U8>(op1.reg) & 0b111; }
    [[nodiscard]] constexpr U8 op2_reg_index() const { return static_cast<U8>(op2.reg) & 0b111; }

    [[nodiscard]] constexpr bool should_compute_address() const { return reg_present || base_present || displacement_present; }

    // TODO : re-enable constexpr when the GCC bug is fixed
    /*constexpr*/ bool operator==(const Inst& other) const = default;
};
