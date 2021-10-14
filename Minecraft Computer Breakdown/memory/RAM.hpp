#pragma once

#include <memory>
#include <memory_resource>

#include "memory_interfaces.hpp"

#include "../ALU.hpp"
#include "StaticBinaryTreeManagedMemory.hpp"


namespace Mem
{

template<U32 N, typename Granularity>
class RAM : public ReadWriteMemoryInterface
{
	static_assert(std::is_integral<Granularity>::value);
	static_assert(N % 8 == 0);

private:
	StaticBinaryTreeManagedMemory<N, sizeof(Granularity)> memory;
	std::pmr::polymorphic_allocator<U8> allocator;

public:
	explicit RAM(U8* const bytes)
		: ReadWriteMemoryInterface(bytes),
		  memory(bytes), allocator(&memory)
	{ }

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

	const StaticBinaryTreeManagedMemory<N, sizeof(Granularity)>& get_memory_manager() const 
	{
		return memory;
	}
};

};
