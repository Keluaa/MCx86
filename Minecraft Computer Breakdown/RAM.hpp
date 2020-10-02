#pragma once

#include <memory>
#include <exception>

#include "data_types.h"

template<U32 N>
struct RAM
{
	static_assert(N % 4 == 0);

	// Separate the memory in 4 sections, to be able to read/write 4 bytes in one step.
	// Memory is accessed with binary trees, one for reading and one for writing, and
	// each memory cell (containing N / 4 bytes) has an isolated tree.
	U8 bytes_1[N / 4];
	U8 bytes_2[N / 4];
	U8 bytes_3[N / 4];
	U8 bytes_4[N / 4];

	RAM()
	{
		// set all bytes to 0
		std::memset(bytes_1, 0, N / 4);
		std::memset(bytes_2, 0, N / 4);
		std::memset(bytes_3, 0, N / 4);
		std::memset(bytes_4, 0, N / 4);
	}
	
	constexpr U8* get_bytes_array(U32 address) const { 
		switch (address % 4) {
		case 0: return bytes_1 + (address >> 2);
		case 1: return bytes_2 + (address >> 2);
		case 2: return bytes_3 + (address >> 2);
		case 3: return bytes_4 + (address >> 2);
		}
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
		case sizeof(U8):
			return *get_bytes_array(address);
		case sizeof(U16):
			return (*get_bytes_array(address + 1) << 8) |
				   (*get_bytes_array(address + 0) << 0);
		case sizeof(U32):
			return (*get_bytes_array(address + 3) << 24) |
				   (*get_bytes_array(address + 2) << 16) |
				   (*get_bytes_array(address + 1) << 8) |
				   (*get_bytes_array(address + 0) << 0);
		default:
			throw std::logic_error("Wrong address size");
		}
	}
	
	template<typename A>
	void write(U32 address, U32 value)
	{
		switch (sizeof(A))
		{
		case sizeof(U8):
			*get_bytes_array(address) = value & 0xFF;
			break;
		case sizeof(U16):
			*get_bytes_array(address + 1) = (value & 0xFF00) >> 8;
			*get_bytes_array(address + 0) = (value & 0x00FF) >> 0;
			break;
		case sizeof(U32):
			*get_bytes_array(address + 3) = (value & 0xFF000000) >> 24;
			*get_bytes_array(address + 2) = (value & 0x00FF0000) >> 16;
			*get_bytes_array(address + 1) = (value & 0x0000FF00) >> 8;
			*get_bytes_array(address + 0) = (value & 0x000000FF) >> 0;
			break;
		default:
			throw std::logic_error("Wrong address size");
		}
	}

	template<typename A>
	A read_and_write(U32 address, A value)
	{
		// in the actual circuit implementation, this is done without temporary variables or anything else.
		A tmp = read<A>(address);
		return tmp;
	}
};
