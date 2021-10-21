#pragma once

#include "memory_interfaces.hpp"

#include "../ALU.hpp"


namespace Mem
{

template<U32 N, typename Granularity>
class Buffer : public ReadWriteMemoryInterface
{
    static_assert(std::is_integral<Granularity>::value);
    static_assert(N % 8 == 0);

public:
    explicit Buffer(U8* const bytes)
            : ReadWriteMemoryInterface(bytes)
    { }
};

}
