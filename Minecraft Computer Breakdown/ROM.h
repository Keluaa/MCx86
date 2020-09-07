#pragma once

#include <memory>

#include "data_types.h"


template<typename CELL_SIZE, unsigned int COUNT>
struct ROM
{
	CELL_SIZE bytes[COUNT];
	
	ROM()
	{
		std::memset(bytes, 0, COUNT);
	}

	void init(const CELL_SIZE* program, unsigned int size)
	{
		// used at initialisation
		std::memcpy(bytes, program, size);
	}

	CELL_SIZE read(unsigned int address) const
	{
		return bytes[address];
	}
};
