#pragma once

#include <memory>
#include <exception>
#include <memory_resource>
#include <cassert>

#include "data_types.h"
#include "instructions.h"

#include "ALU.hpp"
#include "StaticBinaryTreeManagedMemory.hpp"


template<U32 N, typename Granularity>
class RAM
{
	static_assert(std::is_integral<Granularity>::value);
	static_assert(N % 8 == 0);

private:
	U8 bytes[N];
	StaticBinaryTreeManagedMemory<N, sizeof(Granularity)> memory;
	std::pmr::polymorphic_allocator<U8> allocator;

public:
	RAM() : memory(bytes), allocator(&memory)
	{
		std::memset(bytes, 0, N); // TODO : remove if unecessary
	}

	template<class T, class... Args>
	T* allocate(Args&&... args)
	{
		static_assert(std::is_pointer<T*>::value);

		T* t = (T*) allocator.allocate(sizeof(T));
		if (t != nullptr) {
			allocator.construct(t, args...);
		}
		return t;
	};

	template<class T>
	void deallocate(T* const ptr)
	{
		allocator.deallocate((U8*) ptr, sizeof(T));
	}

	U32 read(U32 address, OpSize size) const
	{
		// TODO : bounds check if N < max_U32
		switch (size)
		{
		case OpSize::B:
			return bytes[address];
		case OpSize::W:
			return (bytes[address + 1] << 8) | bytes[address];
		case OpSize::DW:
			return (bytes[address + 3] << 24) |
				   (bytes[address + 2] << 16) |
				   (bytes[address + 1] << 8) |
				    bytes[address];
		default:
			throw std::logic_error("Wrong memory size");
		}
	}

	void write(U32 address, U32 value, OpSize size)
	{
		// TODO : maybe optimize this with some packed assignments
		switch (size)
		{
		case OpSize::B:
			bytes[address] = value & 0xFF;
			break;
		case OpSize::W:
			bytes[address + 1] = (value & 0xFF00) >> 8;
			bytes[address + 0] = (value & 0x00FF) >> 0;
			break;
		case OpSize::DW:
			bytes[address + 3] = (value & 0xFF000000) >> 24;
			bytes[address + 2] = (value & 0x00FF0000) >> 16;
			bytes[address + 1] = (value & 0x0000FF00) >> 8;
			bytes[address + 0] = (value & 0x000000FF) >> 0;
			break;
		default:
			throw std::logic_error("Wrong memory size");
		}
	}

	U32 read_and_write(U32 address, U32 value, OpSize size)
	{
		// in the actual circuit implementation, this is done without temporary variables or anything else.
		U32 tmp = read(address, size);
		write(address, value, size);
		return tmp;
	}

	const StaticBinaryTreeManagedMemory<N, sizeof(Granularity)>& get_memory_manager() const 
	{
		return memory;
	}
};
