﻿#pragma once

#include <vector>

#include "../data_types.h"
#include "exceptions.hpp"


namespace Mem
{

struct Descriptor
{
	U32 base;
	U32 limit:20;
	
	bit granularity:1;
	bit default_size:1;
	bit _long:1;
	bit available:1;
	bit present:1;
	bit dpl:2;
	bit _system:1;
	bit type:3;
	bit accessed:1;
};


struct Selector
{
	
};


struct DescriptorTable
{
	const U32 MAX_SIZE = 8192; // 2^13
	
	std::vector<Descriptor> table;
	
	DescriptorTable()
		: table(100) {}
	
	U32 translate(U16 segment, U32 offset) const
	{
		if (segment >= table.size()) {
			throw BadSelector("Segment out of bounds", segment);
		}
		
		const Descriptor* desc = table.data() + segment;
		
		// bounds check
		U32 size;
		if (desc->granularity) {
			size = (desc->limit << 12) + 0xFFFFF;
		}
		else {
			size = desc->limit;
		}
		
		if (offset >= size) {
			throw BadSelector("Segment offset is bigger than the segment size", segment, size, offset); 
		}
		
		return desc->base + offset;
	}
};

};
