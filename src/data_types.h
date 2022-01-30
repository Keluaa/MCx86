#pragma once

#include <cstdint>

typedef bool bit;

typedef uint8_t  U8;
typedef uint16_t U16;
typedef uint32_t U32;
typedef uint64_t U64; // should be used only internally, not transmitted to other parts of the computer

typedef int8_t  I8;
typedef int16_t I16;
typedef int32_t I32;
typedef int64_t I64; // should not be used


static_assert(sizeof(U8)  == 1, "Invalid architecture, 'char' must be 8 bits");
static_assert(sizeof(U16) == 2, "Invalid architecture, 'short' must be 16 bits");
static_assert(sizeof(U32) == 4, "Invalid architecture, 'int' must be 32 bits");
static_assert(sizeof(U64) == 8, "Invalid architecture, 'long long' must be 64 bits");


/**
 * Size of an operand of an instruction.
 *
 * The most used operand types have the fewest set bits for efficiency.
 */
enum class OpSize : U8
{
    DW = 0b00,      // Double Word (32 bits)
    W  = 0b01,      // Word        (16 bits)
    B  = 0b10,      // Byte        (8  bits)
    UNKNOWN = 0b11
};


/**
 * Type of an operand of an instruction.
 *
 * The most used operand types have the fewest set bits for efficiency.
 */
enum class OpType : U8
{
    REG = 0b00,     // General purpose register
    MEM = 0b01,     // Memory operand
    IMM = 0b10,     // Immediate value
    IMM_MEM = 0b11, // immediate memory address or offset
};
