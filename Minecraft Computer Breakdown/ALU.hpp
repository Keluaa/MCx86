#pragma once

#include <type_traits>

#include "data_types.h"
#include "circuit_branch_monitor.h"
#include "instructions.h"

/*
	TODO: fix description

	Mimics the behaviour of all functionnalities of the ALU, using only the basic operations available.
	Those basic operations include:
	 - single bit shift (right or left) (all bits at once)
	 - and
	 - or
	 - xor
	 - not

	All loops parsing each bit of a integer are unrolled in the circuit for each bit (32 times).
	This means that in order to minimize lag, we must minimize the number of operations done at each iteration.

	In the circuit implementation of the ALU, one sub circuit cannot be used more than once in each clock cycle.
	A branch monitor object is used to make sure we don't try to use a branch more than once in a clock cycle,
	which is called at the start of all functions in the ALU.
	When a ALU function calls another with the flag '_internal_call' set to true, it means that in the circuit
	implementation the entire circuit of the other function is copied next to the caller function's circuit, for
	its own use.
*/

namespace ALU
{
	static CircuitBranchMonitor branchMonitor;

	template<typename N>
	constexpr bit check_equal_zero(N n, bool _internal_call = false)
	{
		static_assert(std::is_integral<N>{}, "check_equal_zero operand must be of integral type");

		if (!_internal_call) USE_BRANCH(branchMonitor);

		typename std::make_unsigned<N>::type mask = 1;
		for (int i = 0; i < sizeof(N) * 8; i++) {
			if (n & mask) {
				return false;
			}
			mask <<= 1;
		}

		return true;
	}


	template<typename N>
	constexpr bit check_different_than_zero(N n, bool _internal_call = false)
	{
		static_assert(std::is_integral<N>{}, "check_different_than_zero operand must be of integral type");

		if (!_internal_call) USE_BRANCH(branchMonitor);

		typename std::make_unsigned<N>::type mask = 1;
		for (int i = 0; i < sizeof(N) * 8; i++) {
			if (n & mask) {
				return true;
			}
		}

		return false;
	}


	template<typename N>
	constexpr bit check_is_negative(N n, OpSize size = OpSize::UNKNOWN, bool _internal_call = false)
	{
		static_assert(std::is_integral<N>{}, "check_is_negative operand must be of integral type");

		if (!_internal_call) USE_BRANCH(branchMonitor);

		U8 last_bit_pos = 0;
		switch (size)
		{
		case OpSize::DW: last_bit_pos = sizeof(U32) * 8 - 1; break;
		case OpSize::W:  last_bit_pos = sizeof(U16) * 8 - 1; break;
		case OpSize::B:  last_bit_pos = sizeof(U8)  * 8 - 1; break;
		default: last_bit_pos = sizeof(N)   * 8 - 1; break;
		}

		// check if the last bit is set
		typename std::make_unsigned<N>::type mask = 1 << last_bit_pos;
		return bool(n & mask);
	}


	/*
		Returns 1 if there is an even number of bits in n, 0 otherwise.
	*/
	template<typename N>
	constexpr bit check_parity(N n, bool _internal_call = false)
	{
		static_assert(std::is_integral<N>{}, "check_parity operand must be of integral type");

		if (!_internal_call) USE_BRANCH(branchMonitor);

		bit res = 0;
		typename std::make_unsigned<N>::type mask = 1;
		for (int i = 0; i < sizeof(N) * 8; i++) {
			res ^= bool(n & mask);
			mask <<= 1;
		}
		return !res;
	}


    template<typename A, typename B>
	constexpr bit compare_equal(A a, B b, bool _internal_call = false)
	{
        static_assert(std::is_integral<A>{} && std::is_integral<B>{}, "compare_equal operands must be of integral type");
		static_assert(sizeof(A) >= sizeof(B), "First operand of 'compare_equal' must have at least the same bit length than the second");

		if (!_internal_call) USE_BRANCH(branchMonitor);

        // we can parse the bits in either direction, but it is more likely to have a difference in the first bits in general.
        typename std::make_unsigned<A>::type mask = 1;

        for (int i = 0; i < sizeof(A) * 8; i++) {
            if (((a & mask) ^ (b & mask)) == 1) {
                return false;
            }
            mask <<= 1;
        }

        return true;
    }


	/*
		Returns a >= b, with an additional flag if a == b.
	*/
	template<typename A, typename B>
	constexpr bit compare_greater_or_equal(A a, B b, bit& equal, bool _internal_call = false)
	{
		static_assert(std::is_integral<A>{} && std::is_integral<B>{}, "compare_greater_or_equal operands must be of integral type");
		static_assert(sizeof(A) >= sizeof(B), "First operand of 'compare_greater_or_equal' must have at least the same bit length than the second");

		if (!_internal_call) USE_BRANCH(branchMonitor);

		typename std::make_unsigned<A>::type mask = 1 << (sizeof(A) * 8 - 1);

		for (int i = 0; i < sizeof(A) * 8; i++) {
			if (((a & mask) ^ (b & mask)) == 0) { // a_i == b_i
				mask >>= 1;
				continue;
			}
			else if (a & mask) { // a_i > b_i
				equal = false;
				return true;
			}
			else { // a_i < b_i
				equal = false;
				return false;
			}
		}

		equal = true;
		return true; // a == b
	}


	template<typename A, typename B>
	constexpr bit compare_greater_or_equal(A a, B b, bool _internal_call = false)
	{
		bit equal = 0;
		return compare_greater_or_equal(a, b, equal, _internal_call);
	}
	
	
	template<typename A, typename B>
	constexpr bit compare_greater(A a, B b, bool _internal_call = false)
	{
		static_assert(std::is_integral<A>{} && std::is_integral<B>{}, "compare_greater operands must be of integral type");
		static_assert(sizeof(A) >= sizeof(B), "First operand of 'compare_greater' must have at least the same bit length than the second");

		if (!_internal_call) USE_BRANCH(branchMonitor);

		typename std::make_unsigned<A>::type mask = 1 << (sizeof(A) * 8 - 1);

		for (int i = 0; i < sizeof(A) * 8; i++) {
			if (((a & mask) ^ (b & mask)) == 0) { // a_i == b_i
				mask >>= 1;
				continue;
			}
			else if (a & mask) { // a_i > b_i
				return true;
			}
			else { // a_i < b_i
				return false;
			}
		}

		return false; // a == b
	}
	
	
	/*
		Sign extend a signed number which has been converted to 
		an unsigned type which is double its size.
	*/
	template<typename N>
	constexpr N sign_extend(N n, OpSize prev_size = OpSize::UNKNOWN, bool _internal_call = false)
	{
		// TODO : check all usages, we should always use prev_size
		static_assert(std::is_integral<N>{}, "sign_extend operand must be of integral type");
		static_assert(std::is_unsigned<N>{}, "sign_extend operand must be unsigned (to prevent auto-extend)");
		
		if (!_internal_call) USE_BRANCH(branchMonitor);

		U8 last_bit_pos = 0;
		switch (prev_size)
		{
		case OpSize::DW: last_bit_pos = sizeof(U32) * 8 / 2 - 1; break; // not useful to have, but hey why not
		case OpSize::W:  last_bit_pos = sizeof(U16) * 8 / 2 - 1; break;
		case OpSize::B:  last_bit_pos = sizeof(U8)  * 8 / 2 - 1; break;
		default: last_bit_pos = sizeof(N)   * 8 / 2 - 1; break; // start at the middle by default
		}
		
		N mask = 1 << last_bit_pos;  
		const bit fill = bool(n & mask);
		mask <<= 1;
		
		if (fill) {
			for (int i = 0; i < sizeof(N) * 8; i++) {
				n |= mask;
				mask <<= 1; 
			}
		}
		
		return n;
	}


	template<typename A, typename B>
	constexpr A and_(A a, B b)
	{
		static_assert(std::is_integral<A>{} && std::is_integral<B>{}, "and operands must be of integral type");
		static_assert(sizeof(A) >= sizeof(B), "First operand of 'and' must have at least the same bit length than the second");

		USE_BRANCH(branchMonitor);

		// flat bitwise operation: each result bits are independant from each other
		return a & b;
	}


	template<typename A, typename B>
	constexpr A or_(A a, B b)
	{
		static_assert(std::is_integral<A>{} && std::is_integral<B>{}, "or operands must be of integral type");
		static_assert(sizeof(A) >= sizeof(B), "First operand of 'or' must have at least the same bit length than the second");

		USE_BRANCH(branchMonitor);

		// flat bitwise operation: each result bits are independant from each other
		return a | b;
	}


	template<typename A, typename B>
	constexpr A xor_(A a, B b)
	{
		static_assert(std::is_integral<A>{} && std::is_integral<B>{}, "xor operands must be of integral type");
		static_assert(sizeof(A) >= sizeof(B), "First operand of 'xor' must have at least the same bit length than the second");

		USE_BRANCH(branchMonitor);

		// flat bitwise operation: each result bits are independant from each other
		return a ^ b;
	}


	template<typename N>
	constexpr N not_(N n)
	{
		static_assert(std::is_integral<N>{}, "not operand must be of integral type");

		USE_BRANCH(branchMonitor);

		// flat bitwise operation: each result bits are independant from each other
		return ~n;
	}
	
	
	template<typename N>
	constexpr N rotate_left_carry(N n, bit& carry, U8 count, OpSize size = OpSize::UNKNOWN)
	{
		static_assert(std::is_integral<N>{}, "rotate_left_carry operand must be of integral type");

		USE_BRANCH(branchMonitor);
		
		U8 last_bit_pos = 0;
		switch (size)
		{
		case OpSize::DW: last_bit_pos = sizeof(U32) * 8 - 1; break;
		case OpSize::W:  last_bit_pos = sizeof(U16) * 8 - 1; break;
		case OpSize::B:  last_bit_pos = sizeof(U8)  * 8 - 1; break;
		default: last_bit_pos = sizeof(N)   * 8 - 1; break;
		}
		typename std::make_unsigned<N>::type mask = 1 << last_bit_pos;
		
		count &= 0b11111; // max 31 rotations
		bit tmp = carry, tmpc = 0;
		for (int i = 0; i < count; i++) {
			tmpc = bool(n & mask);
			n <<= 1;
			n |= N(tmp);
			tmp = tmpc;
		}
		
		carry = tmp;
		return n;
	}
	
	
	template<typename N>
	constexpr N rotate_right_carry(N n, bit& carry, U8 count, OpSize size = OpSize::UNKNOWN)
	{
		static_assert(std::is_integral<N>{}, "rotate_right_carry operand must be of integral type");

		USE_BRANCH(branchMonitor);
		
		U8 last_bit_pos = 0;
		switch (size)
		{
		case OpSize::DW: last_bit_pos = sizeof(U32) * 8 - 1; break;
		case OpSize::W:  last_bit_pos = sizeof(U16) * 8 - 1; break;
		case OpSize::B:  last_bit_pos = sizeof(U8)  * 8 - 1; break;
		default: last_bit_pos = sizeof(N)   * 8 - 1; break;
		}
		
		count &= 0b11111; // max 31 rotations
		bit tmp = carry, tmpc = 0;
		for (int i = 0; i < count; i++) {
			tmpc = bool(n & 0b1);
			n >>= 1;
			n |= tmp << last_bit_pos;
			tmp = tmpc;
		}
		
		carry = tmp;
		return n;
	}
	
	
	template<typename N>
	constexpr N rotate_left(N n, bit& carry, U8 count, OpSize size = OpSize::UNKNOWN)
	{
		static_assert(std::is_integral<N>{}, "rotate_left operand must be of integral type");

		USE_BRANCH(branchMonitor);
		
		U8 last_bit_pos = 0;
		switch (size)
		{
		case OpSize::DW: last_bit_pos = sizeof(U32) * 8 - 1; break;
		case OpSize::W:  last_bit_pos = sizeof(U16) * 8 - 1; break;
		case OpSize::B:  last_bit_pos = sizeof(U8)  * 8 - 1; break;
		default: last_bit_pos = sizeof(N)   * 8 - 1; break;
		}
		typename std::make_unsigned<N>::type mask = 1 << last_bit_pos;
		
		count &= 0b11111; // max 31 rotations
		bit tmp = 0;
		for (int i = 0; i < count; i++) {
			tmp = bool(n & mask);
			n <<= 1;
			n |= N(tmp);
		}
		
		carry = tmp;
		return n;
	}
	
	
	template<typename N>
	constexpr N rotate_right(N n, bit& carry, U8 count, OpSize size = OpSize::UNKNOWN)
	{
		static_assert(std::is_integral<N>{}, "rotate_right operand must be of integral type");

		USE_BRANCH(branchMonitor);
		
		U8 last_bit_pos = 0;
		switch (size)
		{
		case OpSize::DW: last_bit_pos = sizeof(U32) * 8 - 1; break;
		case OpSize::W:  last_bit_pos = sizeof(U16) * 8 - 1; break;
		case OpSize::B:  last_bit_pos = sizeof(U8)  * 8 - 1; break;
		default: last_bit_pos = sizeof(N)   * 8 - 1; break;
		}
		
		count &= 0b11111; // max 31 rotations
		bit tmp = 0;
		for (int i = 0; i < count; i++) {
			tmp = bool(n & 0b1);
			n >>= 1;
			n |= tmp << last_bit_pos;
		}
		
		carry = tmp;
		return n;
	}


    template<typename N>
    constexpr N shift_left(N n, bit& carry, U8 count, OpSize size = OpSize::UNKNOWN)
    {
        static_assert(std::is_integral<N>{}, "shift_left operand must be of integral type");

        USE_BRANCH(branchMonitor);
		
		U8 last_bit_pos = 0;
		switch (size)
		{
		case OpSize::DW: last_bit_pos = sizeof(U32) * 8 - 1; break;
		case OpSize::W:  last_bit_pos = sizeof(U16) * 8 - 1; break;
		case OpSize::B:  last_bit_pos = sizeof(U8)  * 8 - 1; break;
		default: last_bit_pos = sizeof(N)   * 8 - 1; break;
		}
        typename std::make_unsigned<N>::type mask = U64(1) << last_bit_pos; // cast to max precision to prevent any problems

        bit tmp = 0;
        for (int i = 0; i < count; i++) {
            tmp = bool(n & mask);
            n <<= 1;
        }

        carry = tmp;
        return n;
    }


    template<typename N>
    constexpr N shift_right(N n, bit& carry, U8 count, OpSize size = OpSize::UNKNOWN, bit keep_sign = false)
    {
        static_assert(std::is_integral<N>{}, "shift_right operand must be of integral type");

        USE_BRANCH(branchMonitor);

        N sign = 0;
        if (keep_sign) {
            U8 last_bit_pos = 0;
            switch (size)
            {
            case OpSize::DW: last_bit_pos = sizeof(U32) * 8 - 1; break;
            case OpSize::W:  last_bit_pos = sizeof(U16) * 8 - 1; break;
            case OpSize::B:  last_bit_pos = sizeof(U8)  * 8 - 1; break;
            default: last_bit_pos = sizeof(N)   * 8 - 1; break;
            }
            sign = n & (U64(1) << last_bit_pos); // cast to max precision to prevent any problems
        }
		
        bit tmp = 0;
        for (int i = 0; i < count; i++) {
            tmp = bool(n & 0b1);
            n >>= 1;
        }

        n |= sign; // the sign is ORed into the result, to match the behaviour of SAR
        carry = tmp;
        return n;
    }


    template<typename N>
    constexpr N shift_right_no_carry(N n, U8 count, OpSize size = OpSize::UNKNOWN, bit keep_sign = false)
    {
        bit carry = 0;
        return ALU::shift_right(n, carry, count, size, keep_sign);
    }


	template<typename N>
	constexpr U8 get_first_set_bit_index(N n, bit& isZero)
	{
		static_assert(std::is_integral<N>{}, "get_first_set_bit_index operand must be of integral type");

		USE_BRANCH(branchMonitor);

		typename std::make_unsigned<N>::type mask = 1;
		for (int i = 0; i < sizeof(N) * 8; i++) {
			if (n & mask) {
				isZero = false;
				return i;
			}
			mask <<= 1;
		}

		isZero = true;
		return 0;
	}


	template<typename N>
	constexpr U8 get_last_set_bit_index(N n, bit& isZero, bool _internal_call = false)
	{
		static_assert(std::is_integral<N>{}, "get_last_set_bit_index operand must be of integral type");

		if (!_internal_call) USE_BRANCH(branchMonitor);

		typename std::make_unsigned<N>::type mask = 1 << (sizeof(N) * 8 - 1);
		for (int i = sizeof(N) * 8 - 1; i >= 0; i--) {
			if (n & mask) {
				isZero = false;
				return i;
			}
			mask >>= 1;
		}

		isZero = true;
		return 0;
	}


	template<typename N>
	constexpr bit get_and_set_bit_at(N& n, U8 pos, bit val, bool _internal_call = false, bool _no_set = false)
	{
		static_assert(std::is_integral<N>{}, "get_and_set_bit_at operand must be of integral type");
		static_assert(sizeof(N) <= sizeof(U32), "get_and_set_bit_at operand must be a 32 bit type or smaller");

		if (!_internal_call) USE_BRANCH(branchMonitor);

		// This is implemented as a binairy tree in the circuit. 
		// Go left if the i-th bit of pos is set, right otherwise.
		// In the end the bit_mask has only one bit set, which is the target bit.
		// All of this is strictly equivalent to '1 << pos'.
		U32 bit_mask = 0xFFFFFFFF;
		U8 offset = 0;
		for (int i = 4; i >= 0; i--) {
			if (pos & (1 << i)) {
				offset += 1 << i;
				switch (i) {
				case 0: bit_mask &= 0x0001 << offset; break;
				case 1: bit_mask &= 0x0003 << offset; break;
				case 2: bit_mask &= 0x000F << offset; break;
				case 3: bit_mask &= 0x00FF << offset; break;
				case 4: bit_mask &= 0xFFFF << offset; break;
				}
			}
			else {
				switch (i) {
				case 0: bit_mask &= 0x0001 << offset; break;
				case 1: bit_mask &= 0x0003 << offset; break;
				case 2: bit_mask &= 0x000F << offset; break;
				case 3: bit_mask &= 0x00FF << offset; break;
				case 4: bit_mask &= 0xFFFF << offset; break;
				}
			}
		}

		if (!_no_set) {
			n &= ~bit_mask;
			if (val) {
				n |= bit_mask;
			}
		}

		return bit(n & bit_mask);
	}


	template<typename N>
	constexpr bit get_bit_at(const N n, U8 pos, bool _internal_call = false)
	{
		// 'get_bit_at' and 'get_and_set_bit_at' share the same circuit, 
		// because they are very similar, and not much used.
		N n_ = n;
		return get_and_set_bit_at(n_, pos, 0, _internal_call, true);
	}
	
	
	/*
		Addition is implemented so that the sign of the operands doesn't matter.
		This means that the same algorithm can be used for substraction if b < 0.
		Here the algorithm is the pen and paper addition.
	*/
	template<typename A, typename B>
	constexpr A add(A a, B b, bit& carry, bool _internal_call = false)
	{
		static_assert(std::is_integral<A>{} && std::is_integral<B>{}, "Add operands must be of integral type");
		static_assert(sizeof(A) >= sizeof(B), "First operand of 'add' must have at least the same bit length than the second");

		if (!_internal_call) USE_BRANCH(branchMonitor);
		
		A stack = 0;
		typename std::make_unsigned<A>::type mask = 1, tmp;
		typename std::make_unsigned<A>::type _carry = bool(carry);
		for (int i = 0; i < sizeof(A) * 8; i++) {
			if (i != 0) {
				_carry <<= 1;
			}
			tmp = (a & mask) ^ (b & mask);
			stack |= tmp ^ _carry;
			_carry = (a & b & mask) | (tmp & _carry);
			mask <<= 1;
		}

		carry = bool(_carry);
		return stack;
	}
	

	template<typename A, typename B>
	constexpr A add_no_carry(A a, B b, bool _internal_call = false)
	{
		bit carry = 0;
		return add(a, b, carry, _internal_call);
	}
	

	/*
		Returns the two's complement of the input.
		This can be interpreted as negating it.
	*/
	template<typename N>
	constexpr N negate(N n, bool _internal_call = false)
	{
		static_assert(std::is_integral<N>{}, "negate operand must be of integral type");

		if (!_internal_call) USE_BRANCH(branchMonitor);

		N out = 0;
		typename std::make_unsigned<N>::type mask = 1;
		bit negate = false;
		for (int i = 0; i < sizeof(N) * 8; i++) {
			// copy the bits from LSB to MSB until the first one is found,
			// then all the bits are reversed (excluding the first one)
			if (negate) {
				out |= (n & mask) ^ mask; // negate the i-bit of n
			}
			else if (n & mask) {
				negate = true;
				out |= mask;
			}
			mask <<= 1;
		}

		return out;
	}


	template<typename A, typename B>
	constexpr A sub(A a, B b, bit& carry, bool _internal_call = false)
	{
		b = ALU::negate(b, true);
		return ALU::add(a, b, carry, _internal_call);
	}
	
	
	template<typename A, typename B>
	constexpr A sub_no_carry(A a, B b, bool _internal_call = false)
	{
		bit carry = 0;
		b = ALU::negate(b, true);
		return ALU::add(a, b, carry, _internal_call);
	}
	
	
	template<typename N>
	constexpr N abs(N n, bool _internal_call = false)
	{
		static_assert(std::is_integral<N>{}, "abs operand must be of integral type");

		if (!_internal_call) USE_BRANCH(branchMonitor);

		N res;
		if (check_is_negative(n, OpSize::UNKNOWN, true)) {
			res = negate(n, true);
		}
		else {
			res = n;
		}
		
		return res;
	}


	/*
		Pen and paper multiplication.
		Minimises the number of additions performed.
		Shifts are trivial in the circuit implementation, since the loop is unrolled bit lines are shifted in the circuit.
	*/
	template<typename A, typename B>
	constexpr A multiply(A a, B b)
	{
		static_assert(std::is_integral<A>{} && std::is_integral<B>{}, "Multiply operands must be of integral type");
		static_assert(sizeof(A) >= sizeof(B), "First operand of 'multiply' must have at least the same bit length than the second");

		USE_BRANCH(branchMonitor);

		// cast to unsigned with the same bit length for all operands
		typename std::make_unsigned<A>::type a_bits = a;
		typename std::make_unsigned<A>::type b_bits = b;

		typename std::make_unsigned<A>::type stack_bits = 0;
		for (int i = 0; i < sizeof(A) * 8; i++) {
			if (b_bits & 1) {
				stack_bits = add_no_carry(stack_bits, a_bits, true); // stack += a (overflow ignored)
			}
			a_bits <<= 1;
			b_bits >>= 1;
		}

		return stack_bits; // will cast to signed if necessary
	}

	
	/*
		Pen and paper division.
		If the divisor is zero, there is no division and the quotient and remainder are zero.
	*/
	template<typename N, typename D>
	constexpr void unsigned_divide(N n, D d, N& q, N& r, bit& divByZero)
	{
		static_assert(std::is_integral<N>{} && std::is_integral<D>{}, "Division operands must be of integral type");
		static_assert(std::is_unsigned<N>{} && std::is_unsigned<D>{}, "Unsigned Division operands must be unsigned");
		static_assert(sizeof(N) >= sizeof(D), "Dividend must have at least the same bit length of the divisor");

		USE_BRANCH(branchMonitor);
		
		N _d = d; // make sure to have enough space for shifting later
		q = 0;
		
		// This initialisation part searches for the minimum shift needed for an efficient division.
		// Because we use registers with the same size as the operands, we must also do this for the divisor,
		// because if we don't we might substract n with only a fraction of d.
		// This is equivalent to:
		/*
		U8 min_shift_num = get_last_set_bit_index(n) - 1;
		U8 min_shift_denom = sizeof(N) * 8 - get_last_set_bit_index(_d);
		U8 min_shift = min(min_shift_num, min_shift_denom)
		*/
		U8 min_n = 0, min_d = 0xFF; // dummy value 0xFF, used for division by zero check
		N mask = 1 << (sizeof(N) * 8 - 1);
		for (int i = 0; i < sizeof(N) * 8; i++) {
			if (_d & mask) {
				min_d = i;
				break;
			}
			mask >>= 1;
		}

		if (min_d == 0xFF) {
			divByZero = true;
			q = r = 0;
			return;
		}
		else {
			divByZero = false;
		}

		mask = 1 << (sizeof(N) * 8 - 1);
		for (int i = sizeof(N) * 8 - 1; i >= 0; i--) {
			if (n & mask) {
				min_n = i;
				break;
			}
			mask >>= 1;
		}
		
		U8 min_shift = 0;
		if (compare_greater_or_equal(min_n, min_d, true)) {
			min_shift = min_d;
		}
		else {
			min_shift = min_n;
		}

		// Division algorithm. Once again this is the same one as the pen and paper version, but in binairy.
		// We start the computation once we shifted d enough, meaning that all of its bits are available and
		// we reached (or went farther than) the most significant set bit of the numerator. This greatly 
		// reduces the number of operations, as well as making it correct, since in the circuit implementation
		// the loop is unrolled. Divisions like 1 / 2^31 costs as much as 2 / 2.
		// The computation is also ended when we reach a point where n == (d << i), since n = 0 and there is 
		// nothing left to do.
		// Note that here we use 'd << i' for the value of d shifted 'nb bits - i' times to the left, to 
		// preserve all bits of d. In the actual circuit d is shifted to the right once per step.
		bit can_compute = false;
		bit equal = false;
		for (int i = (sizeof(N) * 8 - 1); i >= 0; i--) {
			if (min_shift == i) {
				can_compute = true;
				_d <<= i;
			}

			q <<= 1;

			if (can_compute) {
				if (compare_greater_or_equal(n, _d, equal, true)) {
					n = add_no_carry(n, negate(_d, true)); // n -= d
					q |= 1;

					if (equal) { // n == d
						can_compute = false; // equivalent to break
					}
				}
				_d >>= 1;
			}
		}

		r = n;
	}


	template<typename N, typename D>
	constexpr void signed_divide(N n, D d, N& q, N& r, bit& divByZero)
	{
		static_assert(std::is_integral<N>{} && std::is_integral<D>{}, "Division operands must be of integral type");

		typename std::make_unsigned<N>::type n_unsigned = abs(n, true);
		typename std::make_unsigned<D>::type d_unsigned = abs(d, true);
		
		typename std::make_unsigned<N>::type q_unsigned = 0, r_unsigned = 0;

		unsigned_divide(n_unsigned, d_unsigned, q_unsigned, r_unsigned, divByZero);

		if (divByZero) {
			return;
		}

		q = q_unsigned;
		r = r_unsigned;

		bit sign = (check_is_negative(n, OpSize::UNKNOWN, true)) ^ (check_is_negative(d, OpSize::UNKNOWN, true));
		if (sign) {
			// the result is negative
			q = negate(q, true);
			if (check_different_than_zero(r, true)) {
				r = add_no_carry(d, negate(r, true), true); // reverse the remainder (r = d - r)
			}
		}
	}
}
