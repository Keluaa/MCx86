#pragma once

#include <memory>
#include <vector>

#include "../data_types.h"
#include "../exceptions.h"
#include "RAM.hpp"
#include "ROM.hpp"
#include "stack.hpp"


class WrongMemoryAccess : public ExceptionWithMsg
{
public:
	WrongMemoryAccess(const char* msg, const U32 address) noexcept
	{
		const size_t buffer_size = strlen(msg) + 50;
		char* buffer = new char[buffer_size];
		snprintf(buffer, buffer_size, "Wrong memory access at 0x%x: %s", address, msg);
		this->msg = buffer;
	}
};


class BadSelector : public ExceptionWithMsg
{
public:
	BadSelector(const char* msg, const U16 segment) noexcept
	{
		const size_t buffer_size = strlen(msg) + 20;
		char* buffer = new char[buffer_size];
		snprintf(buffer, buffer_size, "Segment %d: %s", segment, msg);
		this->msg = buffer;
	}
	
	BadSelector(const char* msg, const U16 segment, const U32 size, const U32 offset) noexcept
	{
		const size_t buffer_size = strlen(msg) + 80;
		char* buffer = new char[buffer_size];
		snprintf(buffer, buffer_size, "Segment %d (size: %d), offset %d: %s", segment, size, offset, msg);
		this->msg = buffer;
	}
};


struct Descriptor
{
	U32 base;
	U32 limit:20;
	
	bit granularity:1;
	bit default_size:1;
	bit _long:1;
	bit available:1;
	bit present:1;
	bit dpl:2;
	bit _system:1;
	bit type:3;
	bit accessed:1;
};


struct Selector
{
	
};


struct DescriptorTable
{
	const U32 MAX_SIZE = 8192; // 2^13
	
	std::vector<Descriptor> table;
	
	DescriptorTable()
		: table(100) {}
	
	U32 translate(U16 segment, U32 offset) const
	{
		if (segment >= table.size()) {
			throw BadSelector("Segment out of bounds", segment);
		}
		
		const Descriptor* desc = table.data() + segment;
		
		// bounds check
		U32 size;
		if (desc->granularity) {
			size = (desc->limit << 12) + 0xFFFFF;
		}
		else {
			size = desc->limit;
		}
		
		if (offset >= size) {
			throw BadSelector("Segment offset is bigger than the segment size", segment, size, offset); 
		}
		
		return desc->base + offset;
	}
};


template<U32 ROM_SIZE, U32 RAM_SIZE, U32 STACK_SIZE>
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
		   U32 rom_pos, U32 rom_size, U8* rom_bytes,
		   U32 ram_pos, U32 ram_size, U8* ram_bytes)
		: text_pos(text_pos), text_end(text_pos + text_size),
		  rom_pos(rom_pos), rom_end(rom_pos + ROM_SIZE),
		  ram_pos(ram_pos), ram_end(ram_pos + RAM_SIZE),
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
        // All of those checks can be parallelized
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
