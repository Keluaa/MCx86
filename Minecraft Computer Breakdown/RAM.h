#pragma once

#include <memory>

#include "data_types.h"

template<unsigned int N>
struct RAM
{
	U8 bytes[N];

	RAM()
	{
		std::memset(bytes, 0, N);
	}

	U8 read(unsigned int address) const
	{
		return bytes[address];
	}

	
	void write(unsigned int address, U8 value)
	{
		bytes[address] = value;
	}

	U8 read_and_write(unsigned int address, U8 value)
	{
		// in the actual circuit implementation, this is done without temporary variables or anything else.
		U8 tmp = bytes[address];
		bytes[address] = value;
		return tmp;
	}
};
