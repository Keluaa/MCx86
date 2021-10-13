#pragma once

#include "data_types.h"


template<typename CELL_SIZE>
struct ROM
{
private:
    const U32 size;
    const CELL_SIZE* const bytes;

public:
    ROM(const CELL_SIZE* const bytes, U32 size)
        : size(size), bytes(bytes)
    { }

	CELL_SIZE read(U32 address) const
	{
		return bytes[address];
	}
};
