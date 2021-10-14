#pragma once

#include "memory_interfaces.hpp"


namespace Mem
{

template<U32 SIZE>
struct ROM : public ReadMemoryInterface
{
public:
	explicit ROM(U8* bytes)
		: ReadMemoryInterface(bytes)
	{ }
};

};
