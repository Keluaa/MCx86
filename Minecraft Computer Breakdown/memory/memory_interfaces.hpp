#pragma once

#include <stdexcept>

#include "../data_types.h"
#include "../instructions.h"


namespace Mem
{

class ReadMemoryInterface
{
protected:
	U8* const bytes;

	explicit ReadMemoryInterface(U8* const bytes)
		: bytes(bytes)
	{ }

public:
	[[nodiscard]]
	U32 read(U32 address, OpSize size) const
	{
		// TODO : bounds check if N < max_U32
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
			
		default:
			throw std::logic_error("Wrong memory size");
		}
	}

    [[nodiscard]]
    U8* get_bytes() const { return bytes; }
};


class ReadWriteMemoryInterface : public ReadMemoryInterface
{
protected:
	explicit ReadWriteMemoryInterface(U8* const bytes)
		: ReadMemoryInterface(bytes)
	{ }
	
public:
	void write(U32 address, U32 value, OpSize size)
	{
		// TODO : maybe optimize this with some packed assignments
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
	U32 read_and_write(U32 address, U32 value, OpSize size)
	{
		// In the actual circuit implementation, this is done without temporary variables or anything else.
		// This is possible by design.
		U32 tmp = read(address, size);
		write(address, value, size);
		return tmp;
	}
};

}
