﻿#pragma once

#include "memory_interfaces.hpp"


template<U32 SIZE>
class Stack : public ReadWriteMemoryInterface
{
public:
	explicit Stack(U8* bytes)
		: ReadWriteMemoryInterface(bytes)
	{ }
};
