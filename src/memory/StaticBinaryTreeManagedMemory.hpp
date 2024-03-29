﻿#pragma once

#include "../data_types.h"
#include "../ALU.hpp"
#include "../CPU/exceptions.h"


namespace Mem
{

/**
 * This static allocator structure is limited to 2^16 cells (15 layers)
 * because this implementation is not adapted for very large memory management.
 * 
 * However smarter implementations should be able to manage bigger amounts of
 * memory. It is not made here because the purpose of this is to have a good
 * representation of how the circuit implementation works.
 */
template<U8 layer>
struct TreeCell
{
	static_assert(layer > 1);
	static_assert(layer < 16);

	const U16 cell_size;

	const U8* const memory_position;

	TreeCell<layer - 1>* const right;
	TreeCell<layer - 1>* const left;

	const U32 mask;

	/**
	 * Keeps track of which cells layers are available.
	 * Each bit represent a layer, set following this rule:
	 *    0 -> allocatable
	 *    1 -> non-allocatable
	 * 
	 * 'alloc_slots' is a combination of the 'alloc_slots' of the children of 
	 * the cell, following this rule:
	 *  - all bits of the children slots are ANDed together
	 *  - one useful bit after the ones used by the children is added, and set
	 *	  from an OR of the last bits of the child slots
	 * 
	 * This means that if the i-th bit of 'alloc_slots' is set, then no
	 * allocation of a cell in the i-th layer can be made from this cell.
	 * Each change of state is propagated to all parents of the cell.
	 */
	U16 alloc_slots;
	U16 right_slots;
	U16 left_slots;

    /**
     * Returns the size in bytes which the children cells and below occupies in memory.
     */
    static constexpr U32 tree_cells_below_size();

    /**
     * Here we use placement-news to build the tree without the need for actual allocation for each of the cells.
     * This also means that our destructor can be trivial since by freeing the bytes where the tree is stored all of the
     * cells are destroyed in the process.
     *
     * The cells are arranged in a specific way to simplify their creation: by knowing how many bytes are occupied by
     * the cells below the one we are creating, we can build the tree with the following structure:
     *  - For a depth of 3 layers:
     *         3 2 1 1 2 1 1
     *  - For a depth of 4 layers:
     *       4 3 2 1 1 2 1 1 3 2 1 1 2 1 1
     *  - For a depth of 5 layers:
     *     5 4 3 2 1 1 2 1 1 3 2 1 1 2 1 1 4 3 2 1 1 2 1 1 3 2 1 1 2 1 1
     * etc...
     */
    TreeCell(U8* tree_bytes, U16 cell_size, const U8* const memory_position)
            : cell_size(cell_size), memory_position(memory_position),
              right(new (tree_bytes)
                        TreeCell<layer - 1>(tree_bytes + sizeof(TreeCell<layer>),
                                            cell_size / 2, memory_position)),
              left(new (tree_bytes + tree_cells_below_size() / 2)
                        TreeCell<layer - 1>(tree_bytes + sizeof(TreeCell<layer>) + tree_cells_below_size() / 2,
                                            cell_size / 2, memory_position + cell_size / 2)),
              mask((1 << (layer + 1)) - 1), alloc_slots(0), right_slots(0), left_slots(0)
    { }

    const U8* allocate_for(U8 alloc_size);
	void deallocate(U32 cell_index, U8 target_layer);
	void update_alloc_slots();
};


template<>
struct TreeCell<1>
{
	const U32 cell_size;

    const U8* const memory_position;

	const U32 mask;
	U32 alloc_slots;
	bit right_slot;

    TreeCell(U8*, U32 cell_size, const U8* const memory_position)
            : cell_size(cell_size), memory_position(memory_position),
              mask(0b11), alloc_slots(0), right_slot(0)
    { }

	// Did you know? Explicit template class methods must be marked with inline if they use a different definition
    inline const U8* allocate_for(U8 alloc_size);
    inline void deallocate(U32 cell_index, U8 target_layer);
};


template<U8 layer>
constexpr U32 get_cell_allocated_size(const TreeCell<layer>* cell);


template<U32 memory_size, U8 granularity>
class StaticBinaryTreeManagedMemory
{
    static_assert(ALU::check_power_of_2(granularity));
    static_assert(ALU::check_power_of_2(memory_size));
	static_assert(memory_size >= 8); // memory_size must be in bits, not bytes!
	static_assert(memory_size >= granularity * 8); // granularity must be in bytes, I know it is confusing

	static constexpr U8 get_max_layer();
	static constexpr U32 compute_cells_count();

    /**
     * Returns the power of 2 of the number of cells needed to place the number of bytes in memory. This number is the
     * level at which the allocation with take place.
     * 'is_zero' is set if 'bytes' is zero.
     */
	static constexpr U8 get_allocation_size(size_t bytes, bit& is_zero);

public:
    /**
     * Returns the number of bytes taken by the cells of the tree. For debug only.
     */
    static constexpr U32 get_tree_cells_size();

	explicit StaticBinaryTreeManagedMemory(const U8* const memory_position)
		: tree_bytes(new U8[get_tree_cells_size()]),
          memory_position(memory_position),
		  layers_count(get_max_layer()), cells_count(compute_cells_count()),
		  allocator_tree_root(tree_bytes, granularity << layers_count, memory_position)
	{ }

	~StaticBinaryTreeManagedMemory()
    {
        delete[] tree_bytes;
    }

    [[maybe_unused, nodiscard]] constexpr U32 get_memory_size()  const { return memory_size;  }
	[[maybe_unused, nodiscard]] constexpr U8  get_granularity()  const { return granularity;  }
	[[maybe_unused, nodiscard]] constexpr U32 get_cells_count()  const { return cells_count;  }
	[[maybe_unused, nodiscard]] constexpr U32 get_layers_count() const { return layers_count; }

    /**
     * Returns the number of bytes currently allocated. For debug only.
     */
    [[maybe_unused, nodiscard]] U32 get_allocated_memory_size() const;

    [[nodiscard]] void* allocate(size_t bytes);
    void deallocate(void* ptr, size_t bytes);

private:
    U8* const tree_bytes;
    const U8* const memory_position;
	const U8 layers_count;
	const U32 cells_count;
	TreeCell<get_max_layer()> allocator_tree_root;
};


// ============================
// ------ Implementation ------
// ============================


template<U32 memory_size, U8 granularity>
constexpr U32 StaticBinaryTreeManagedMemory<memory_size, granularity>::compute_cells_count()
{
    static_assert((memory_size / (8 * granularity)) > 1);
	return memory_size / (8 * granularity);
}


template<U32 memory_size, U8 granularity>
constexpr U8 StaticBinaryTreeManagedMemory<memory_size, granularity>::get_max_layer()
{
	return ALU::get_last_set_bit_index_no_zero(compute_cells_count());
}


template<U32 memory_size, U8 granularity>
constexpr U8 StaticBinaryTreeManagedMemory<memory_size, granularity>::get_allocation_size(size_t bytes, bit& is_zero)
{
	constexpr U8 cell_bytes_pow = ALU::get_last_set_bit_index_no_zero(granularity);

	// Make 'bytes_pow' such that 2^bytes_pow >= bytes
	U8 bytes_pow = ALU::get_last_set_bit_index(bytes, is_zero);
	if (is_zero) {
		return 0;
	}

	if (!ALU::check_power_of_2(bytes)) {
		bytes_pow = ALU::add_no_carry(bytes_pow, U8(1));
	}

	// Now get the 2^cells_pow number of cells needed for this allocation
	U8 cells_pow = ALU::sub_no_carry(bytes_pow, cell_bytes_pow);
	if (ALU::check_is_negative(cells_pow, OpSize::B)) {
		cells_pow = 0; // case where bytes < granularity
	}

	return cells_pow;
}


template<U32 memory_size, U8 granularity>
constexpr U32 StaticBinaryTreeManagedMemory<memory_size, granularity>::get_tree_cells_size()
{
    constexpr U32 base_cell_size = sizeof(TreeCell<1>);
    constexpr U32 derived_cell_size = sizeof(TreeCell<2>);
    constexpr U32 cells_count = compute_cells_count();

    // base_cell_size * cells_count + derived_cell_size * (cells_count / 2 + cells_count / 4 + ... + cells_count / 2^max_layer)
    // With (cells_count / 2 + cells_count / 4 + ... + cells_count / 2^max_layer) = 2^(max_layer+1) - 1 - cells_count

    U32 derived_cells_count = (cells_count << 1) - 1 - cells_count;
    return base_cell_size * cells_count + derived_cells_count * derived_cell_size;
}


template<U8 layer>
constexpr U32 get_cell_allocated_size(const TreeCell<layer>* cell)
{
	if (cell->alloc_slots == (0b10 << layer) - 1) {
		return cell->cell_size;
	}
	else if (cell->alloc_slots == 0) {
		return 0;
	}
	else {
		return get_cell_allocated_size(cell->right) + get_cell_allocated_size(cell->left);
	}
}


template<>
constexpr U32 get_cell_allocated_size(const TreeCell<1>* cell)
{
	if (cell->alloc_slots == 0b11) {
		return cell->cell_size;
	}
	else if (cell->alloc_slots == 0) {
		return 0;
	}
	else {
		return cell->cell_size / 2;
	}
}


template<U32 memory_size, U8 granularity>
U32 StaticBinaryTreeManagedMemory<memory_size, granularity>::get_allocated_memory_size() const
{
	// Parse through the tree and count the number of bytes allocated
	// This is intended to be only used for testing purposes
	return get_cell_allocated_size(&allocator_tree_root);
}


template<U32 memory_size, U8 granularity>
void* StaticBinaryTreeManagedMemory<memory_size, granularity>::allocate(size_t bytes)
{
	// alloc_size is the layer index where we want to allocate a cell
	bit is_zero = 0;
	U8 alloc_size = get_allocation_size(bytes, is_zero);
	if (is_zero) {
		return nullptr; // nothing to allocate
	}
	
	// check if we have enough space to allocate this
	if (ALU::compare_greater(alloc_size, layers_count)) {
		return nullptr; // we don't have enough memory for this allocation
	}

	if (ALU::get_bit_at(allocator_tree_root.alloc_slots, alloc_size)) {
		return nullptr; // no space in memory for this allocation
	}

	// there is enough space in memory for this allocation, now perform it.
	return (void*) allocator_tree_root.allocate_for(alloc_size);
}


template<U32 memory_size, U8 granularity>
void StaticBinaryTreeManagedMemory<memory_size, granularity>::deallocate(void* ptr, size_t bytes)
{
	static constexpr U32 ptr_mask = U32(U64(U32(-1)) << granularity);

	if (ptr == nullptr) {
		return;
	}

	// find the cell which was allocated for this pointer

	// Get the 'true' address in the memory we manage, in order to check if it is a correctly allocated pointer.
	// Here we cast to U64 to not have any precision loss, but we use 32-bit addressing in the circuit implementation.
	const U32 effective_address = U32(U64(ptr) - U64(memory_position)); 
	if (ALU::check_different_than_zero(ALU::and_(effective_address, ptr_mask))) {
		WARNING("Invalid pointer deallocation");
		return; // This pointer couldn't have been allocated by us, since it is not a multiple of the granularity
	}

	// alloc_size is the layer index where the pointer was allocated
	bit is_zero = 0;
	U8 alloc_size = get_allocation_size(bytes, is_zero);
	if (is_zero) {
		WARNING("Invalid pointer deallocation");
		return;
	}

	U8 cell_size = ALU::add_no_carry(alloc_size, granularity);
	U32 cell_index = ALU::shift_right_no_carry(effective_address, cell_size, OpSize::DW);
	U32 remainder = (effective_address << cell_size) >> cell_size; // remainder from the previous shift operation

	if (ALU::check_different_than_zero(remainder)) {
		WARNING("Invalid pointer deallocation");
		return; // The pointer is not at the start of the expected cell
	}

	allocator_tree_root.deallocate(cell_index, alloc_size);
}


template<U8 layer>
constexpr U32 TreeCell<layer>::tree_cells_below_size()
{
    // Layer 1: 2^(layer-1) cells of size sizeof(TreeCell<1>)
    // Layer 2 and above: Sum of 2^(layer-i) from i=2 to layer-1 cells of size sizeof(TreeCell<2>),
    //                    which simplifies to 2^(layer-1) - 2 cells
    return sizeof(TreeCell<2>) * ((1 << (layer - 1)) - 2) + sizeof(TreeCell<1>) * (1 << (layer - 1));
}


template<U8 layer>
const U8* TreeCell<layer>::allocate_for(U8 alloc_size)
{
	if (ALU::compare_equal(alloc_size, layer)) {
		// allocate for the size of this cell
		alloc_slots = mask;

		return memory_position;
	}
	else {
		// delegate the allocation to the children
        const U8* alloc_pos;
		if (((right_slots >> alloc_size) & 0b1) == 0) {
			alloc_pos = right->allocate_for(alloc_size);
		}
		else {
			alloc_pos = left->allocate_for(alloc_size);
		}

		update_alloc_slots();
		return alloc_pos;
	}
}


const U8* TreeCell<1>::allocate_for(U8 alloc_size)
{
    if (alloc_size == 0b1) {
        // allocate the whole cell
        alloc_slots = mask;

        return memory_position;
    }
    else {
        // simplification for the first layer, since there is no children here, and there must be a parent
        if (right_slot == 0) {
            // allocate the right cell
            right_slot = 1;

            if (alloc_slots == 0b10) {
                alloc_slots = mask; // the left cell is also allocated, this cell is full
            }
            else {
                alloc_slots = 0b10;
            }

            return memory_position;
        }
        else {
            // allocate the left cell, which makes this cell filled
            alloc_slots = mask;

            return memory_position + cell_size / 2;
        }
    }
}


template<U8 layer>
void TreeCell<layer>::deallocate(U32 cell_index, U8 target_layer)
{
	if (ALU::compare_equal(layer, target_layer)) {
		// we are at the cell we want to deallocate
		alloc_slots = 0;
	}
	else {
		if (((cell_index >> layer) & 0b1) == 0b1) {
			left->deallocate(cell_index, target_layer);
		} 
		else {
			right->deallocate(cell_index, target_layer);
		}
		update_alloc_slots();
	}
}


void TreeCell<1>::deallocate(U32 cell_index, U8 target_layer)
{
    if ((target_layer & 0b1) == 0b1) {
        // we are at the cell we want to deallocate
        alloc_slots = 0b00;
    }
    else {
        // we want to deallocate a cell at the layer 0
        if ((cell_index & 0b1) == 0b0) {
            right_slot = 0; // deallocate the right cell
        }

        // deallocate the child cell, while taking into account that the other child cell might be still allocated
        alloc_slots = right_slot << 1;
    }
}


template<U8 layer>
void TreeCell<layer>::update_alloc_slots()
{
	// we only need to fetch the value of the child who called, but we do both for simplicity
	right_slots = right->alloc_slots;
	left_slots = left->alloc_slots;

	alloc_slots = ALU::and_(right_slots, left_slots);

	// the new useful bit of 'alloc_slots' is and OR on the previous last bits of the children cells
	alloc_slots |= ALU::or_(right_slots & (0b1 << (layer - 1)), left_slots & (0b1 << (layer - 1))) << 1;
}

}
