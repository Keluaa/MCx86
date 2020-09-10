#pragma once

#include <memory>

#include "data_types.h"


template<typename CELL_SIZE, U32 SIZE>
struct ROM
{
private:
	CELL_SIZE bytes[SIZE];

public:
	ROM()
	{
		std::memset(bytes, 0, SIZE);
	}

	void init(const CELL_SIZE* program, unsigned int size)
	{
		// used only at initialisation
		std::memcpy(bytes, program, size);
	}

	CELL_SIZE read(unsigned int address) const
	{
		return bytes[address];
	}
};
