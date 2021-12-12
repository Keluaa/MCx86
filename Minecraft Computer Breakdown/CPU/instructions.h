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
 * It is not encoded, which simplifies the circuitry by a lot, however this comes at the cost of bigger memory usage,
 * but by design this is not a problem. The only downside is that the executable files are bigger.
 *
 * Total size: 107 bits used.
 */
struct Inst
{
	U8 opcode;

    /**
     * Struct describing an operand.
     *
     * If the operand is a memory operand, the 'reg' field can hold more info: the first 3 bits are the base register
     * index and the 2 last bits are the scale of the scaled register.
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

    /*
        Addressing

    In IA-32, the following addressing modes are possible (in 32 bit addressing):
      mod r/m index  base               effective address
      11  ...                                    base reg
      00  101                                               displacement
      00  ...                                    base reg
      ..  ...                                    base reg + displacement
      ..  100  100   ...                         base reg + displacement
      ..  100  ...   ...    scaled reg * scale + base reg + displacement
      00  100  ...   101    scaled reg * scale +          + displacement
      ..  100  ...   101    scaled reg * scale +   EBP    + displacement

    The base register is always a 32 bit register index, unless Mod = 11, in which case it is an operand and so scaled
    according to the operand size overrides.
    The scaled register is always a 32 bit register index.
    Displacement is a signed value of either 8 bits or 32 bits.
    The scale is encoded in 2 bits as follows: '0b00' -> 1, '0b01' -> 2, '0b10' -> 4, '0b11' -> 8

    Here we encode those addressing modes this way:
      - base reg     : stored as a register index in the 3 low bits of the 'reg' field of the operand of type memory
      - scaled reg   : stored explicitly in the 'scaled_reg' field
      - scale        : stored in the high 2 bits of the 'reg' field of the operand of type memory
      - displacement : stored as a 32 bit value. 8 bit displacements are sign-extended to 32 bit.

    Then to compute the address, the following flags are used to indicate the presence of a field :
      - base_reg_present   : if the 'reg' field of the memory operand has a register index
      - scaled_reg_present : if the 'scaled_reg' field has a register index

    If 'compute_address' is true, then it is computed according to the following formula. Any absent field is replaced
    by zero.
                base_reg + scaled_reg * scale + displacement

    If 'compute_address' is false, then only the base register is loaded.
     */
    bit compute_address : 1;
    bit base_reg_present : 1;
    bit scaled_reg_present : 1;
    U8 scaled_reg : 3;
	
	U8 : 0; // alignment

	U32 address_value; // Holds an address displacement value or an immediate constant address
	U32 immediate_value;

    Inst() = default;

    // The Operand struct by itself cannot describe when it is not used, here is how to do it for both operands:
    [[nodiscard]] constexpr bool is_op1_none() const { return !op1.read && !write_ret1_to_op1 && op1.type == OpType::REG; }
    [[nodiscard]] constexpr bool is_op2_none() const { return !op2.read && !write_ret2_to_op2 && op2.type == OpType::REG; }

    [[nodiscard]] constexpr U8 op1_reg_index() const { return static_cast<U8>(op1.reg) & 0b111; }
    [[nodiscard]] constexpr U8 op2_reg_index() const { return static_cast<U8>(op2.reg) & 0b111; }

    // TODO : re-enable constexpr when the GCC bug is fixed
    /*constexpr*/ bool operator==(const Inst& other) const = default;
};
