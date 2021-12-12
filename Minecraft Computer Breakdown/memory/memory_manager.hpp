#pragma once

#include <memory>

#include "../data_types.h"
#include "exceptions.hpp"
#include "descriptor_table.hpp"
#include "RAM.hpp"
#include "ROM.hpp"
#include "stack.hpp"


namespace Mem
{

// 1MB of ROM, 1MB of RAM, 1MB of stack
const U32 ROM_SIZE = 0x100000;
const U32 RAM_SIZE = 0x100000;
const U32 STACK_SIZE = 0x100000;

class Memory
{
	// TODO : branch multi-access checking

public:
	const U32 text_pos, text_end;
	const U32 rom_pos, rom_end;
	const U32 ram_pos, ram_end;
	const U32 stack_pos, stack_end;

private:
	std::unique_ptr<U8[]> rom_bytes;
	std::unique_ptr<U8[]> ram_bytes;
	std::unique_ptr<U8[]> stack_bytes;
	
	ROM<ROM_SIZE> rom;
	RAM<RAM_SIZE, U32> ram;
	Stack<STACK_SIZE> stack;
	
	const std::vector<Inst> instructions;
	
public:
	/**
	 * The memory manager takes ownership of the ROM and RAM data,
	 * as well as all of the instructions.
	 */
	Memory(U32 text_pos, U32 text_size,
		   U32 rom_pos, U8* rom_bytes, U8* ram_bytes,
		   std::vector<Inst>& instructions)
		: text_pos(text_pos), text_end(text_pos + text_size),
		  rom_pos(rom_pos), rom_end(rom_pos + ROM_SIZE),
		  ram_pos(rom_end), ram_end(ram_pos + RAM_SIZE),
		  stack_pos(ram_end), stack_end(stack_pos + STACK_SIZE),
		  rom_bytes(rom_bytes), ram_bytes(ram_bytes),
          stack_bytes(std::make_unique<U8[]>(STACK_SIZE)),
		  rom(rom_bytes),
		  ram(ram_bytes),
		  stack(stack_bytes.get()),
		  instructions(std::move(instructions))
	{ }
	
	Memory(const Memory&) = delete;
	Memory& operator=(const Memory&) = delete;

    ROM<ROM_SIZE>* get_ROM()                    { return &rom; }
    RAM<RAM_SIZE, U32>* get_RAM()               { return &ram; }
    Stack<STACK_SIZE>* get_stack()              { return &stack; }
    const std::vector<Inst>* get_instructions() { return &instructions; }

    // TODO : low-level logic for memory accesses

    [[nodiscard]]
    const Inst& fetch_instruction(U32 address) const
    {
        return instructions.at(address - text_pos);
    }

	[[nodiscard]]
    U8* physical_at(U32 address) const
	{
		// All of those checks can be parallelized
		if (address >= text_pos && address < text_end) {
			throw WrongMemoryAccess("Text is read-only.", address);
		}
		else if (address >= rom_pos && address < rom_end) {
			return rom_bytes.get() + (address - rom_pos);
		}
		else if (address >= ram_pos && address < ram_end) {
			return ram_bytes.get() + (address - ram_pos);
		}
		else if (address >= stack_pos && address < stack_end) {
			return stack_bytes.get() + (address - stack_pos);
		}
		else {
			throw WrongMemoryAccess("Address out of bounds.", address);
		}
	}

    [[nodiscard]]
    U32 read(U32 address, OpSize size) const
    {
        // All of those checks can be parallelized using bit checks at the right places
        if (address >= text_pos && address < text_end) {
            throw WrongMemoryAccess("Text cannot be read.", address);
        }
        else if (address >= rom_pos && address < rom_end) {
            return rom.read(address - rom_pos, size);
        }
        else if (address >= ram_pos && address < ram_end) {
            return ram.read(address - ram_pos, size);
        }
        else if (address >= stack_pos && address < stack_end) {
            return stack.read(address - stack_pos, size);
        }
        else {
            throw WrongMemoryAccess("Address out of bounds.", address);
        }
    }

    void write(U32 address, U32 value, OpSize size)
    {
        // All of those checks can be parallelized using bit checks at the right places
        if (address >= text_pos && address < text_end) {
            throw WrongMemoryAccess("Text cannot be written to.", address);
        }
        else if (address >= rom_pos && address < rom_end) {
            throw WrongMemoryAccess("ROM is read-only.", address);
        }
        else if (address >= ram_pos && address < ram_end) {
            ram.write(address - ram_pos, value, size);
        }
        else if (address >= stack_pos && address < stack_end) {
            stack.write(address - stack_pos, value, size);
        }
        else {
            throw WrongMemoryAccess("Address out of bounds.", address);
        }
    }
};

}
