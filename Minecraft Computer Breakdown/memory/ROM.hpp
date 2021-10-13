﻿#pragma once

#include "memory_interfaces.hpp"


template<U32 SIZE>
struct ROM : public ReadMemoryInterface
{
public:
	ROM(U8* bytes)
		: ReadMemoryInterface(bytes)
	{ }
};
