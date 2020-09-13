#pragma once

#include <memory>

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

	U8 read(U32 address) const
	{
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
