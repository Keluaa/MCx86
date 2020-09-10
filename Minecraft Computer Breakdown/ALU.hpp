#pragma once

#include <cmath>
#include <type_traits>

#include "data_types.h"


/*
	Mimics the behaviour of all functionnalities of the ALU, using only the basic operations available.
	Those basic operations include:
	 - single bit shift (right or left) (all bits at once)
	 - and
	 - or
	 - xor
	 - not

	All loops parsing each bit of a integer are unrolled in the circuit for each bit (32 times).
	This means that in order to minimize lag, we must minimize the number of operations done at each iteration.
*/

namespace ALU 
{

	template<typename N>
	constexpr bit check_equal_zero(N n)
	{
		static_assert(std::is_integral<N>{}, "check_equal_zero operand must be of integral type");

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
	constexpr bit check_different_than_zero(N n)
	{
		static_assert(std::is_integral<N>{}, "check_different_than_zero operand must be of integral type");

		typename std::make_unsigned<N>::type mask = 1;
		for (int i = 0; i < sizeof(N) * 8; i++) {
			if (n & mask) {
				return true;
			}
		}

		return false;
	}


	template<typename N>
	constexpr bit check_is_negative(N n)
	{
		static_assert(std::is_integral<N>{}, "check_is_negative operand must be of integral type");
		// check if the last bit is set
		typename std::make_unsigned<N>::type mask = 1 << (sizeof(N) * 8 - 1);
		return bool(n & mask);
	}
	
	template<typename N>
	constexpr bit check_parity(N n)
	{
		static_assert(std::is_integral<N>{}, "check_parity operand must be of integral type");
		
		bit res = 0;
		typename std::make_unsigned<N>::type mask = 1;
		for(int i = 0; i < sizeof(N) * 8; i++) {
			res ^= bool(n & mask);
		}
		return res;
	}

	/*
		Returns a >= b, with an additional flag if a == b.		
	*/
	template<typename A, typename B>
	constexpr bit compare_greater_or_equal(A a, B b, bit& equal)
	{
		static_assert(std::is_integral<A>{} && std::is_integral<B>{}, "compare_greater_or_equal operands must be of integral type");
		static_assert(sizeof(A) >= sizeof(B), "First operand of 'compare_greater_or_equal' must have at least the same bit length than the second");

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
	constexpr bit compare_greater_or_equal(A a, B b)
	{
		bit equal = 0;
		return compare_greater_or_equal(a, b, equal);
	}

	/*
		Addition is implemented so that the sign of the operands doesn't matter.
		This means that the same algorithm can be used for substraction if b < 0.
		Here the algorithm is the pen and paper addition.
	*/
	template<typename A, typename B>
	constexpr A add(A a, B b, bit& carry)
	{
		static_assert(std::is_integral<A>{} && std::is_integral<B>{}, "Multiply operands must be of integral type");
		static_assert(sizeof(A) >= sizeof(B), "First operand of 'multiply' must have at least the same bit length than the second");

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
	constexpr A add_no_carry(A a, B b)
	{
		bit carry = 0;
		return add(a, b, carry);
	}
	

	/*
		Returns the two's complement of the input.
		This can be interpreted as negating it.
	*/
	template<typename N>
	constexpr N negate(N n)
	{
		static_assert(std::is_integral<N>{}, "negate operand must be of integral type");

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

		// cast to unsigned with the same bit length for all operands
		typename std::make_unsigned<A>::type a_bits = a;
		typename std::make_unsigned<A>::type b_bits = b;

		typename std::make_unsigned<A>::type stack_bits = 0;
		for (int i = 0; i < sizeof(A) * 8; i++) {
			if (b_bits & 1) {
				stack_bits = add_no_carry(stack_bits, a_bits); // stack += a (overflow ignored)
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

		if (check_equal_zero(d)) {
			divByZero = true;
			q = r = 0;
			return;
		}
		else {
			divByZero = false;
		}

		N _d = d; // make sure to have enough space for shifting later

		q = 0;
		
		// This initialisation part searches for the minimum shift needed for an efficient division.
		// Because we use registers with the same size as the operands, we must also do this for the divisor,
		// because if we don't we might substract n with only a fraction of d.
		// This is equivalent to:
		/*
		int min_shift_num = get_most_significant_set_bit(n) - 1;
		int min_shift_denom = sizeof(N) * 8 - get_most_significant_set_bit(_d);
		int min_shift = min(min_shift_num, min_shift_denom)
		*/
		// with 'get_most_significant_set_bit' implemented the same way as for the loop over n:
		N mask = 1 << (sizeof(N) * 8 - 1);
		unsigned int min_n = 0, min_d = 0;
		for (int i = sizeof(N) * 8 - 1; i >= 0; i--) {
			if (n & mask) {
				min_n = i;
				break;
			}
			mask >>= 1;
		}
		mask = 1 << (sizeof(N) * 8 - 1);
		for (int i = 0; i < sizeof(N) * 8; i++) {
			if (_d & mask) {
				min_d = i;
				break;
			}
			mask >>= 1;
		}

		unsigned int min_shift = 0;
		if (compare_greater_or_equal(min_n, min_d)) {
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
				if (compare_greater_or_equal(n, _d, equal)) {
					n = add_no_carry(n, negate(_d)); // n -= d
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

		typename std::make_unsigned<N>::type n_unsigned = std::abs(n);
		typename std::make_unsigned<D>::type d_unsigned = std::abs(d);
		
		typename std::make_unsigned<N>::type q_unsigned, r_unsigned;

		unsigned_divide(n_unsigned, d_unsigned, q_unsigned, r_unsigned, divByZero);

		if (divByZero) {
			return;
		}

		q = q_unsigned;
		r = r_unsigned;

		bit sign = (check_is_negative(n)) ^ (check_is_negative(d));
		if (sign) {
			// the result is negative
			q = negate(q);
			if (check_different_than_zero(r)) {
				r = add_no_carry(d, negate(r)); // reverse the remainder (r = d - r)
			}
		}
	}
}
