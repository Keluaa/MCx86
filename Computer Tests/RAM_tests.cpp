
#include "doctest.h"

#include "memory/RAM.hpp"


TEST_SUITE("RAM_alloc")
{
    TEST_CASE("alloc_U32")
    {
        RAM<128, U32> ram;

        REQUIRE(ram.get_memory_manager().get_cells_count() == 4);
        REQUIRE(ram.get_memory_manager().get_layers_count() == 2);

        U32* test = ram.allocate<U32>(42);

        REQUIRE(test != nullptr);
        REQUIRE(*test == 42);
        REQUIRE(ram.get_memory_manager().get_allocated_memory_size() == sizeof(U32));

        ram.deallocate(test);

        REQUIRE(ram.get_memory_manager().get_allocated_memory_size() == 0);
    }

    TEST_CASE("cell_1_alloc_fail")
    {
        //    0
        //  0   0
        // 0 0 0 0
        RAM<128, U32> ram;

        //    0
        //  1   0
        // 0 0 0 0
        U64* double_cell = ram.allocate<U64>(42);

        REQUIRE(double_cell != nullptr);
        REQUIRE(*double_cell == 42);
        REQUIRE(ram.get_memory_manager().get_allocated_memory_size() == sizeof(U64));

        //    0
        //  1   0
        // 0 0 1 0
        U32* single_cell_1 = ram.allocate<U32>(1000);

        REQUIRE(single_cell_1 != nullptr);
        REQUIRE(*single_cell_1 == 1000);
        REQUIRE(*double_cell == 42);
        REQUIRE(ram.get_memory_manager().get_allocated_memory_size() == (sizeof(U64) + sizeof(U32)));

        //    0
        //  0   0
        // 0 0 1 0
        ram.deallocate(double_cell);

        REQUIRE(ram.get_memory_manager().get_allocated_memory_size() == sizeof(U32));

        //    0
        //  0   0
        // 1 0 1 0
        U32* second_single = ram.allocate<U32>(1010);

        REQUIRE(second_single != nullptr);
        REQUIRE(*second_single == 1010);
        REQUIRE(*single_cell_1 == 1000);
        REQUIRE(ram.get_memory_manager().get_allocated_memory_size() == (2 *sizeof(U32)));

        //    0
        //  0   0
        // 1 0 1 0 <- no space to allocate 2 consecutive cells
        double_cell = ram.allocate<U64>(99);

        REQUIRE(double_cell == nullptr);
        REQUIRE(*second_single == 1010);
        REQUIRE(*single_cell_1 == 1000);
        REQUIRE(ram.get_memory_manager().get_allocated_memory_size() == (2 * sizeof(U32)));
    }

    TEST_CASE("full_alloc")
    {
        RAM<64, U8> ram;

        REQUIRE(ram.get_memory_manager().get_cells_count() == 8);
        REQUIRE(ram.get_memory_manager().get_layers_count() == 3);

        //        1        <- allocate the entire ram in one allocation
        //    0       0
        //  0   0   0   0
        // 0 0 0 0 0 0 0 0
        U64* big_number = ram.allocate<U64>(0xC0FF'EE00'DEAD'4269);

        REQUIRE(big_number != nullptr);
        REQUIRE(*big_number == 0xC0FF'EE00'DEAD'4269);
        REQUIRE(ram.get_memory_manager().get_allocated_memory_size() == sizeof(U64));

        U8* small_boi = ram.allocate<U8>(0xDC); // <- no more space

        REQUIRE(small_boi == nullptr);

        ram.deallocate(big_number);

        REQUIRE(ram.get_memory_manager().get_allocated_memory_size() == 0);
    }

    TEST_CASE("over_alloc")
    {
        RAM<32, U16> ram;

        REQUIRE(ram.get_memory_manager().get_cells_count() == 2);
        REQUIRE(ram.get_memory_manager().get_layers_count() == 1);

        U64* big_number = ram.allocate<U64>(0); // not enough space

        REQUIRE(big_number == nullptr);
        REQUIRE(ram.get_memory_manager().get_allocated_memory_size() == 0);
    }

    TEST_CASE("under_alloc")
    {
        RAM<64, U16> ram;

        REQUIRE(ram.get_memory_manager().get_cells_count() == 4);
        REQUIRE(ram.get_memory_manager().get_layers_count() == 2);

        //    0
        //  0   0
        // 1 0 0 0 <- U8 is smaller than one cell, but takes an entire one anyway
        U8* small_number = ram.allocate<U8>(42);

        REQUIRE(small_number != nullptr);
        REQUIRE(*small_number == 42);
        REQUIRE(ram.get_memory_manager().get_allocated_memory_size() == sizeof(U16));

        //    0
        //  0   0
        // 1 1 0 0
        U16* neighbour = ram.allocate<U16>(75);

        REQUIRE(small_number != nullptr);
        REQUIRE(*neighbour == 75);
        REQUIRE(*small_number == 42);
        REQUIRE(ram.get_memory_manager().get_allocated_memory_size() == (2 * sizeof(U16)));
        REQUIRE_EQ(U64(small_number) + 2 * sizeof(U8), U64(neighbour)); // both pointers should be neighbours
    }
}
