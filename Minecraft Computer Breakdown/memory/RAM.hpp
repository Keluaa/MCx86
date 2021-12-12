#pragma once

#include <memory>

#include "memory_interfaces.hpp"

#include "../ALU.hpp"
#include "StaticBinaryTreeManagedMemory.hpp"


namespace Mem
{

template<U32 N, typename Granularity>
class RAM : public ReadWriteMemoryInterface
{
	static_assert(std::is_integral_v<Granularity>);
	static_assert(N % 8 == 0);

private:
	StaticBinaryTreeManagedMemory<N, sizeof(Granularity)> memory;

public:
	explicit RAM(U8* bytes)
		: ReadWriteMemoryInterface(bytes),
		  memory(bytes)
	{ }

    [[maybe_unused, nodiscard]] constexpr U32 get_size() const { return N; }
    [[maybe_unused, nodiscard]] constexpr U32 get_granularity() const { return sizeof(Granularity); }

	template<class T, class... Args>
	T* allocate(Args&&... args)
	{
        T* t = static_cast<T*>(memory.allocate(sizeof(T)));
		if (t != nullptr) {
#ifndef __clang__
            std::construct_at(t, std::forward<Args>(args)...);
#else
            // Don't use fancy STL allocation functions for compatibility with Clang
            ::new (const_cast<void*>(static_cast<const volatile void*>(t))) T(std::forward<Args>(args)...);
#endif
		}
		return t;
	}

	template<class T>
	void deallocate(T* const ptr)
	{
        memory.deallocate((U8*) ptr, sizeof(T));
	}

	const auto& get_memory_manager() const { return memory; }
};

}
