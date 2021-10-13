#pragma once

#include <stdexcept>

#include "data_types.h"
#include "instructions.h"


template<U32 SIZE>
struct Stack
{
	U8* const bytes;

public:
	Stack(U8* bytes)
		: bytes(bytes)
	{ }
	
	void write(U32 address, U32 value, OpSize size)
	{
		switch (size)
		{
		case OpSize::B:
			bytes[address] = value & 0xFF;
			break;
			
		case OpSize::W:
			bytes[address + 1] = (value & 0xFF00) >> 8;
			bytes[address + 0] = (value & 0x00FF) >> 0;
			break;
			
		case OpSize::DW:
			bytes[address + 3] = (value & 0xFF000000) >> 24;
			bytes[address + 2] = (value & 0x00FF0000) >> 16;
			bytes[address + 1] = (value & 0x0000FF00) >>  8;
			bytes[address + 0] = (value & 0x000000FF) >>  0;
			break;
			
		default:
			throw std::logic_error("Wrong memory size");
		}
	}

	[[nodiscard]] 
	U32 read(U32 address, OpSize size) const
	{
		switch (size)
		{
		case OpSize::B:
			return bytes[address];
		
		case OpSize::W:
			return (bytes[address + 1] << 8) | 
					bytes[address];
		
		case OpSize::DW:
			return (bytes[address + 3] << 24) |
				   (bytes[address + 2] << 16) |
				   (bytes[address + 1] <<  8) |
				    bytes[address];
		
		case OpSize::UNKNOWN:
			throw std::logic_error("Wrong memory size");
		}
	}
};
