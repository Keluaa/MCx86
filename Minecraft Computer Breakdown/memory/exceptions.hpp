#pragma once

#include "../data_types.h"
#include "../exceptions.h"


namespace Mem
{

class WrongMemoryAccess : public ExceptionWithMsg
{
public:
	WrongMemoryAccess(const char* msg, const U32 address) noexcept
	{
		const size_t buffer_size = strlen(msg) + 50;
		char* buffer = new char[buffer_size];
		snprintf(buffer, buffer_size, "Wrong memory access at 0x%x: %s", address, msg);
		this->msg = buffer;
	}
};


class BadSelector : public ExceptionWithMsg
{
public:
	BadSelector(const char* msg, const U16 segment) noexcept
	{
		const size_t buffer_size = strlen(msg) + 20;
		char* buffer = new char[buffer_size];
		snprintf(buffer, buffer_size, "Segment %d: %s", segment, msg);
		this->msg = buffer;
	}
	
	BadSelector(const char* msg, const U16 segment, const U32 size, const U32 offset) noexcept
	{
		const size_t buffer_size = strlen(msg) + 80;
		char* buffer = new char[buffer_size];
		snprintf(buffer, buffer_size, "Segment %d (size: %d), offset %d: %s", segment, size, offset, msg);
		this->msg = buffer;
	}
};

};
