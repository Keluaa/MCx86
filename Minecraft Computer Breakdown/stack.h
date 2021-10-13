#pragma once

#include "data_types.h"


/**
 * Stack object
 */
class Stack
{
    const U32 size;
    U8* const bytes;

public:
    Stack(U8* const bytes, U32 size) : size(size), bytes(bytes) { }

	[[nodiscard]] const U8* get_bytes() const { return bytes; }

    U32 read(U32 address, OpSize value_size)
    {
        switch (value_size) {
        case OpSize::B:
            return bytes[address];

        case OpSize::W:
            return (bytes[address + 1] << 8) | bytes[address];

        case OpSize::DW:
            return (bytes[address + 3] << 24) |
                   (bytes[address + 2] << 16) |
                   (bytes[address + 1] <<  8) |
                    bytes[address];

        default:
            throw std::logic_error("Wrong stack size");
        }
    }

    void write(U32 address, U32 value, OpSize value_size)
    {
        // TODO : I have a doubt on the endianness
        switch (value_size) {
        case OpSize::W:
            bytes[address + 1] = (value & 0xFF00) >> 8;
            bytes[address + 0] = (value & 0x00FF) >> 0;
            break;

        case OpSize::DW:
            bytes[address + 3] = (value & 0xFF000000) >> 24;
            bytes[address + 2] = (value & 0x00FF0000) >> 16;
            bytes[address + 1] = (value & 0x0000FF00) >> 8;
            bytes[address + 0] = (value & 0x000000FF) >> 0;
            break;

        default:
            // Can't push single bytes to the stack
            throw std::logic_error("Wrong stack size");
        }
    }
};
