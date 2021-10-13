#pragma once

#include <memory>

#include "data_types.h"


template<U32 SIZE>
struct ROM
{
private:
	U8* const bytes;

public:
	ROM(U8* bytes)
		: bytes(bytes)
	{ }

	constexpr U8 read(U32 address) const
	{
		return bytes[address];
	}
};
