#pragma once

#include <memory>
#include <exception>

#include "data_types.h"

template<U32 N>
struct RAM
{
	U8 bytes[N];

	RAM()
	{
		std::memset(bytes, 0, N);
	}

	U32 to_physical_address(U16 segment, U32 address)
	{
		// TODO : descriptors tables, global and local, etc...  (p. 95)
	}

	template<typename A>
	A read(U32 address) const
	{
		switch (sizeof(A))
		{
		case 1:
			return bytes[address];
		case 2:
			return bytes[address + 1] << 8 + bytes[address];
		case 4:
			return bytes[address + 3] << 24 + bytes[address + 2] << 16 +
				   bytes[address + 1] << 8 + bytes[address];
		default:
			throw std::logic_error("Wrong address size");
		}
		return bytes[address];
	}
	
	void write(U32 address, U8 value)
	{
		bytes[address] = value;
	}

	U8 read_and_write(U32 address, U8 value)
	{
		// in the actual circuit implementation, this is done without temporary variables or anything else.
		U8 tmp = bytes[address];
		bytes[address] = value;
		return tmp;
	}
};
