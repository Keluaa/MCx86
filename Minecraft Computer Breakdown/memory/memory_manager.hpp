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

// 2MB of ROM, 4MB of RAM, 2MB of stack
const U32 ROM_SIZE = 0x200000;
const U32 RAM_SIZE = 0x400000;
const U32 STACK_SIZE = 0x200000;

class Memory
{
	U32 text_pos, text_end;
	U32 rom_pos, rom_end; 
	U32 ram_pos, ram_end;
	U32 stack_pos, stack_end;
	
	std::unique_ptr<U8> rom_bytes;
	std::unique_ptr<U8> ram_bytes;
	std::unique_ptr<U8> stack_bytes;
	
	ROM<ROM_SIZE> rom;
	RAM<RAM_SIZE, U32> ram;
	Stack<STACK_SIZE> stack;
	
public:
	/**
	 * The memory manager takes ownership of the ROM and RAM data.
	 */
	Memory(U32 text_pos, U32 text_size,
		   U32 rom_pos, U8* rom_bytes, U8* ram_bytes)
		: text_pos(text_pos), text_end(text_pos + text_size),
		  rom_pos(rom_pos), rom_end(rom_pos + ROM_SIZE),
		  ram_pos(rom_end), ram_end(ram_pos + RAM_SIZE),
		  stack_pos(ram_end), stack_end(stack_pos + STACK_SIZE),
		  rom_bytes(rom_bytes), ram_bytes(ram_bytes),
		  stack_bytes(std::make_unique<U8>(STACK_SIZE)),
		  rom(rom_bytes),
		  ram(ram_bytes),
		  stack(stack_bytes.get())
	{
		if (rom_size > ROM_SIZE) {
			std::cout << "Error: given ROM size (" << rom_size << ") is greater than the maximum one (" << ROM_SIZE << ").\n";
		}
		
		if (ram_size > RAM_SIZE) {
			std::cout << "Error: given RAM size (" << ram_size << ") is greater than the maximum one (" << RAM_SIZE << ").\n";
		}
	}
	
	Memory(const Memory&) = delete;
	Memory& operator=(const Memory&) = delete;
	
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
    U32 at(U32 address, OpSize size) const
    {
        // All of those checks can be parallelized using bit checks at the right places
        if (address >= text_pos && address < text_end) {
            throw WrongMemoryAccess("Text is read-only.", address);
        }
        else if (address >= rom_pos && address < rom_end) {
            return rom.read(address - rom_pos, size);
        }
        else if (address >= ram_pos && address < ram_end) {
            return ram.read(address - ram_pos, size);
        }
        else if (address >= stack_pos && address < stack_end) {
            return stack.read(address - stack_pos);
        }
        else {
            throw WrongMemoryAccess("Address out of bounds.", address);
        }
    }
};

};
