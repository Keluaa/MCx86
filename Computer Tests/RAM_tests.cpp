#include "stdafx.h"
#include "CppUnitTest.h"

#include "RAM_2.hpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace ComputerTests
{
	TEST_CLASS(RAM_alloc)
	{
	public:

		TEST_METHOD(alloc_U32)
		{
			RAM<128, U32> ram;

			Assert::IsTrue(ram.get_memory_manager().get_cells_count() == 4);
			Assert::IsTrue(ram.get_memory_manager().get_layers_count() == 2);

			U32* test = ram.allocate<U32>(42);

			Assert::IsNotNull(test);
			Assert::IsTrue(*test == 42);
			Assert::IsTrue(ram.get_memory_manager().get_allocated_memory_size() == sizeof(U32));

			ram.deallocate(test);

			Assert::IsTrue(ram.get_memory_manager().get_allocated_memory_size() == 0);
		}

		TEST_METHOD(cell_1_alloc_fail)
		{
			//    0 
			//  0   0
			// 0 0 0 0
			RAM<128, U32> ram;

			//    0 
			//  1   0
			// 0 0 0 0
			U64* double_cell = ram.allocate<U64>(42);

			Assert::IsNotNull(double_cell);
			Assert::IsTrue(*double_cell == 42);
			Assert::IsTrue(ram.get_memory_manager().get_allocated_memory_size() == sizeof(U64));

			//    0 
			//  1   0
			// 0 0 1 0
			U32* single_cell_1 = ram.allocate<U32>(1000);

			Assert::IsNotNull(single_cell_1);
			Assert::IsTrue(*single_cell_1 == 1000);
			Assert::IsTrue(*double_cell == 42);
			Assert::IsTrue(ram.get_memory_manager().get_allocated_memory_size() == (sizeof(U64) + sizeof(U32)));

			//    0 
			//  0   0
			// 0 0 1 0
			ram.deallocate(double_cell);

			Assert::IsTrue(ram.get_memory_manager().get_allocated_memory_size() == sizeof(U32));

			//    0 
			//  0   0
			// 1 0 1 0
			U32* second_single = ram.allocate<U32>(1010);

			Assert::IsNotNull(second_single);
			Assert::IsTrue(*second_single == 1010);
			Assert::IsTrue(*single_cell_1 == 1000);
			Assert::IsTrue(ram.get_memory_manager().get_allocated_memory_size() == (2 *sizeof(U32)));

			//    0 
			//  0   0  
			// 1 0 1 0 <- no space to allocate 2 consecutive cells
			double_cell = ram.allocate<U64>(99);

			Assert::IsNull(double_cell);
			Assert::IsTrue(*second_single == 1010);
			Assert::IsTrue(*single_cell_1 == 1000);
			Assert::IsTrue(ram.get_memory_manager().get_allocated_memory_size() == (2 * sizeof(U32)));
		}
		
		TEST_METHOD(full_alloc)
		{
			RAM<64, U8> ram;

			Assert::IsTrue(ram.get_memory_manager().get_cells_count() == 8);
			Assert::IsTrue(ram.get_memory_manager().get_layers_count() == 3);

			//        1        <- allocate the entire ram in one allocation
			//    0       0
			//  0   0   0   0
			// 0 0 0 0 0 0 0 0
			U64* big_number = ram.allocate<U64>(0xC0FF'EE00'DEAD'4269);

			Assert::IsNotNull(big_number);
			Assert::IsTrue(*big_number == 0xC0FF'EE00'DEAD'4269);
			Assert::IsTrue(ram.get_memory_manager().get_allocated_memory_size() == sizeof(U64));

			U8* small_boi = ram.allocate<U8>(0xDC); // <- no more space

			Assert::IsNull(small_boi);

			ram.deallocate(big_number);

			Assert::IsTrue(ram.get_memory_manager().get_allocated_memory_size() == 0);
		}

		TEST_METHOD(over_alloc)
		{
			RAM<32, U16> ram;

			Assert::IsTrue(ram.get_memory_manager().get_cells_count() == 2);
			Assert::IsTrue(ram.get_memory_manager().get_layers_count() == 1);

			U64* big_number = ram.allocate<U64>(0); // not enough space

			Assert::IsNull(big_number);
			Assert::IsTrue(ram.get_memory_manager().get_allocated_memory_size() == 0);
		}

		TEST_METHOD(under_alloc)
		{
			RAM<64, U16> ram;

			Assert::IsTrue(ram.get_memory_manager().get_cells_count() == 4);
			Assert::IsTrue(ram.get_memory_manager().get_layers_count() == 2);

			//    0
			//  0   0
			// 1 0 0 0 <- U8 is smaller than one cell, but takes an entire one anyway
			U8* small_number = ram.allocate<U8>(42);

			Assert::IsNotNull(small_number);
			Assert::IsTrue(*small_number == 42);
			Assert::IsTrue(ram.get_memory_manager().get_allocated_memory_size() == sizeof(U16));

			//    0
			//  0   0
			// 1 1 0 0
			U16* neighbour = ram.allocate<U16>(75);

			Assert::IsNotNull(small_number);
			Assert::IsTrue(*neighbour == 75);
			Assert::IsTrue(*small_number == 42);
			Assert::IsTrue(ram.get_memory_manager().get_allocated_memory_size() == (2 * sizeof(U16)));
			Assert::AreEqual(U64(small_number) + 2 * sizeof(U8), U64(neighbour)); // both pointers should be neighbours
		}
	};
};
