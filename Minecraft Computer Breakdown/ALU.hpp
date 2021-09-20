#pragma once

#include <type_traits>

#include "data_types.h"
#include "circuit_branch_monitor.h"
#include "instructions.h"


/**
 * TODO: fix description
 *
 * Mimics the behaviour of all functionalities of the ALU, using only the basic operations available.
 * Those basic operations include:
 *   - single bit shift (right or left) (one, several or all bits at once, exchanging bits is also possible)
 *   - and
 *   - or
 *   - xor
 *   - not
 *
 * All loops parsing each bit of a integer are unrolled in the circuit for each bit (32 times).
 * This means that in order to minimize lag, we must minimize the number of operations done at each iteration.
 *
 * This is why it is preferable to use binary trees to parse an integer. 'Flat' implementations are even better.
 */
namespace ALU
{
    template<typename N>
    constexpr U8 get_last_bit_pos(const OpSize size)
    {
        U8 last_bit_pos;
        switch (size)
        {
        case OpSize::DW: last_bit_pos = sizeof(U32) * 8 - 1; break;
        case OpSize::W:  last_bit_pos = sizeof(U16) * 8 - 1; break;
        case OpSize::B:  last_bit_pos = sizeof(U8)  * 8 - 1; break;
        default:         last_bit_pos = sizeof(N)   * 8 - 1; break;
        }

        return last_bit_pos;
    }


    template<typename N>
    constexpr U8 get_half_bit_pos(const OpSize size)
    {
        U8 half_bit_pos;
        switch (size)
        {
        case OpSize::DW: half_bit_pos = sizeof(U32) * 8 / 2 - 1; break;
        case OpSize::W:  half_bit_pos = sizeof(U16) * 8 / 2 - 1; break;
        case OpSize::B:  half_bit_pos = sizeof(U8)  * 8 / 2 - 1; break;
        default:         half_bit_pos = sizeof(N)   * 8 / 2 - 1; break;
        }

        return half_bit_pos;
    }


    template<typename N>
    consteval U8 get_binary_tree_depth()
    {
        static_assert(std::is_integral<N>::value, "N is not an integral type");

        U8 loops_count;
        switch (sizeof(N)) {
        case sizeof(U8):  loops_count = 3; break;
        case sizeof(U16): loops_count = 4; break;
        case sizeof(U32): loops_count = 5; break;
        case sizeof(U64): loops_count = 6; break;
        default:          loops_count = 0; break;
        }

        return loops_count;
    }


    template<typename N>
    constexpr bit check_equal_zero(const N n)
    {
        static_assert(std::is_integral<N>{}, "check_equal_zero operand must be of integral type");

        // I have now found a very good circuit implementation which computes this result instantly without any explicit
        // logic gates. This function will stay however.
        return !bool(n);
    }


    template<typename N>
    constexpr bit check_different_than_zero(const N n)
    {
        static_assert(std::is_integral<N>{}, "check_different_than_zero operand must be of integral type");

        // I have now found a very good circuit implementation which computes this result instantly without any explicit
        // logic gates. This function will stay however.
        return bool(n);
    }


    template<typename N>
    constexpr bit check_is_negative(const N n, const OpSize size = OpSize::UNKNOWN)
    {
        static_assert(std::is_integral<N>{}, "check_is_negative operand must be of integral type");

        const U8 last_bit_pos = get_last_bit_pos<N>(size);

        // check if the last bit is set
        typename std::make_unsigned<N>::type mask = 1 << last_bit_pos;
        return bool(n & mask);
    }


    /**
     *	@brief Returns 1 if there is an even number of bits in n, 0 otherwise.
     */
    template<typename N>
    constexpr bit check_parity(const N n)
    {
        static_assert(std::is_integral<N>{}, "check_parity operand must be of integral type");

        bit res = 0;
        typename std::make_unsigned<N>::type mask = 1;
        for (int i = 0; i < sizeof(N) * 8; i++) {
            res ^= bool(n & mask);
            mask <<= 1;
        }
        return !res;
    }


    /**
     * @brief Returns 1 if there is only one set bit in n, else returns 0
     */
    template<typename N>
    constexpr bit check_power_of_2(const N n)
    {
        static_assert(std::is_integral<N>{}, "check_power_of_2 operand must be of integral type");
        static_assert(std::is_unsigned<N>{}, "check_power_of_2 operand must be an unsigned type");

        bit one_encountered = 0;
        N mask = 1;
        for (int i = 0; i < sizeof(N) * 8; i++) {
            if (mask & n) {
                if (one_encountered) {
                    return false;
                }
                else {
                    one_encountered = 1;
                }
            }
            mask <<= 1;
        }

        return one_encountered;
    }


    template<typename A, typename B>
    constexpr bit compare_equal(const A a, const B b)
    {
        static_assert(std::is_integral<A>{} && std::is_integral<B>{}, "compare_equal operands must be of integral type");
        static_assert(sizeof(A) >= sizeof(B), "First operand of 'compare_equal' must have at least the same bit length than the second");

        // we can parse the bits in either direction, but it is more likely to have a difference in the first bits in general.
        typename std::make_unsigned<A>::type mask = 1;

        for (int i = 0; i < sizeof(A) * 8; i++) {
            if (((a & mask) ^ (b & mask)) != 0) {
                return false;
            }
            mask <<= 1;
        }

        return true;
    }


    /**
     * @brief Returns a >= b, with an additional flag if a == b.
     */
    template<typename A, typename B>
    constexpr bit compare_greater_or_equal_with_eq(const A a, const B b, bit& equal)
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
    constexpr bit compare_greater_or_equal(const A a, const B b)
    {
        bit equal = 0;
        return compare_greater_or_equal_with_eq(a, b, equal);
    }


    template<typename A, typename B>
    constexpr bit compare_greater(const A a, const B b)
    {
        static_assert(std::is_integral<A>{} && std::is_integral<B>{}, "compare_greater operands must be of integral type");
        static_assert(sizeof(A) >= sizeof(B), "First operand of 'compare_greater' must have at least the same bit length than the second");

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


    /**
     *	@brief extend a signed number which has been converted to an unsigned type which is double its size.
     */
    template<typename N>
    constexpr N sign_extend(const N n, const OpSize prev_size = OpSize::UNKNOWN)
    {
        // TODO : check all usages, we should always use prev_size
        static_assert(std::is_integral<N>{}, "sign_extend operand must be of integral type");
        static_assert(std::is_unsigned<N>{}, "sign_extend operand must be unsigned (to prevent auto-extend)");

        const U8 half_bit_pos = get_half_bit_pos<N>(prev_size);
        N mask = 1 << half_bit_pos;
        const bit fill = bool(n & mask);
        mask <<= 1;

        N out = n;

        if (fill) {
            for (int i = 0; i < sizeof(N) * 8; i++) {
                out |= mask;
                mask <<= 1;
            }
        }

        return out;
    }


    template<typename A, typename B>
    constexpr A and_(const A a, const B b)
    {
        static_assert(std::is_integral<A>{} && std::is_integral<B>{}, "and operands must be of integral type");
        static_assert(sizeof(A) >= sizeof(B), "First operand of 'and' must have at least the same bit length than the second");

        // flat bitwise operation: each result bits are independent from each other
        return a & b;
    }


    template<typename A, typename B>
    constexpr A or_(const A a, const B b)
    {
        static_assert(std::is_integral<A>{} && std::is_integral<B>{}, "or operands must be of integral type");
        static_assert(sizeof(A) >= sizeof(B), "First operand of 'or' must have at least the same bit length than the second");

        // flat bitwise operation: each result bits are independent from each other
        return a | b;
    }


    template<typename A, typename B>
    constexpr A xor_(const A a, const B b)
    {
        static_assert(std::is_integral<A>{} && std::is_integral<B>{}, "xor operands must be of integral type");
        static_assert(sizeof(A) >= sizeof(B), "First operand of 'xor' must have at least the same bit length than the second");

        // flat bitwise operation: each result bits are independent from each other
        return a ^ b;
    }


    template<typename N>
    constexpr N not_(const N n)
    {
        static_assert(std::is_integral<N>{}, "not operand must be of integral type");

        // flat bitwise operation: each result bits are independent from each other
        return ~n;
    }


    template<typename N>
    constexpr N rotate_left_carry(const N n, bit& carry, U8 count, const OpSize size = OpSize::UNKNOWN)
    {
        static_assert(std::is_integral<N>{}, "rotate_left_carry operand must be of integral type");
        static_assert(sizeof(N) <= sizeof(U32), "rotate_left_carry does not support rotations of types bigger than 32 bits");

        const U8 last_bit_pos = get_last_bit_pos<N>(size);
        typename std::make_unsigned<N>::type mask = 1 << last_bit_pos;

        count &= 0b11111; // max 31 rotations
        N stack = n;
        bit tmp = carry, tmp_c;
        for (int i = 0; i < count; i++) {
            tmp_c = bool(stack & mask);
            stack <<= 1;
            stack |= N(tmp);
            tmp = tmp_c;
        }

        carry = tmp;
        return stack;
    }


    template<typename N>
    constexpr N rotate_right_carry(const N n, bit& carry, U8 count, const OpSize size = OpSize::UNKNOWN)
    {
        static_assert(std::is_integral<N>{}, "rotate_right_carry operand must be of integral type");
        static_assert(sizeof(N) <= sizeof(U32), "rotate_right_carry does not support rotations of types bigger than 32 bits");

        const U8 last_bit_pos = get_last_bit_pos<N>(size);

        count &= 0b11111; // max 31 rotations
        N stack = n;
        bit tmp = carry, tmp_c;
        for (int i = 0; i < count; i++) {
            tmp_c = bool(stack & 0b1);
            stack >>= 1;
            stack |= tmp << last_bit_pos;
            tmp = tmp_c;
        }

        carry = tmp;
        return stack;
    }


    template<typename N>
    constexpr N rotate_left(const N n, bit& carry, U8 count, const OpSize size = OpSize::UNKNOWN)
    {
        static_assert(std::is_integral<N>{}, "rotate_left operand must be of integral type");
        static_assert(sizeof(N) <= sizeof(U32), "rotate_left does not support rotations of types bigger than 32 bits");

        const U8 last_bit_pos = get_last_bit_pos<N>(size);
        typename std::make_unsigned<N>::type mask = 1 << last_bit_pos;

        count &= 0b11111; // max 31 rotations
        N stack = n;
        bit tmp = 0;
        for (int i = 0; i < count; i++) {
            tmp = bool(stack & mask);
            stack <<= 1;
            stack |= N(tmp);
        }

        carry = tmp;
        return stack;
    }


    template<typename N>
    constexpr N rotate_right(const N n, bit& carry, U8 count, const OpSize size = OpSize::UNKNOWN)
    {
        static_assert(std::is_integral<N>{}, "rotate_right operand must be of integral type");
        static_assert(sizeof(N) <= sizeof(U32), "rotate_right does not support rotations of types bigger than 32 bits");

        const U8 last_bit_pos = get_last_bit_pos<N>(size);

        count &= 0b11111; // max 31 rotations
        N stack = n;
        bit tmp = 0;
        for (int i = 0; i < count; i++) {
            tmp = bool(stack & 0b1);
            stack >>= 1;
            stack |= tmp << last_bit_pos;
        }

        carry = tmp;
        return stack;
    }


    template<typename N>
    constexpr N shift_left(const N n, bit& carry, U8 count, const OpSize size = OpSize::UNKNOWN)
    {
        static_assert(std::is_integral<N>{}, "shift_left operand must be of integral type");

        const U8 last_bit_pos = get_last_bit_pos<N>(size);
        typename std::make_unsigned<N>::type mask = U64(1) << last_bit_pos; // cast to max precision to prevent any problems

        N stack = n;
        bit tmp = 0;
        for (int i = 0; i < count; i++) {
            tmp = bool(stack & mask);
            stack <<= 1;
        }

        carry = tmp;
        return stack;
    }


    template<typename N>
    constexpr N shift_left_no_carry(const N n, U8 count, const OpSize size = OpSize::UNKNOWN)
    {
        bit _ = 0;
        return shift_left(n, _, count, size);
    }


    template<typename N>
    constexpr N shift_right(const N n, bit& carry, U8 count, const OpSize size = OpSize::UNKNOWN, const bit keep_sign = false)
    {
        static_assert(std::is_integral<N>{}, "shift_right operand must be of integral type");

        N sign = 0;
        if (keep_sign) {
            const U8 last_bit_pos = get_last_bit_pos<N>(size);
            sign = n & (U64(1) << last_bit_pos); // cast to max precision to prevent any problems
        }

        N stack = n;
        bit tmp = 0;
        for (int i = 0; i < count; i++) {
            tmp = bool(stack & 0b1);
            stack >>= 1;
        }

        stack |= sign; // the sign is OR'ed into the result, to match the behaviour of SAR
        carry = tmp;
        return stack;
    }


    template<typename N>
    constexpr N shift_right_no_carry(const N n, U8 count, const OpSize size = OpSize::UNKNOWN, const bit keep_sign = false)
    {
        bit carry = 0;
        return ALU::shift_right(n, carry, count, size, keep_sign);
    }


    template<typename N>
    constexpr U8 get_first_set_bit_index(const N n, bit& is_zero)
    {
        static_assert(std::is_integral<N>{}, "get_first_set_bit_index operand must be of integral type");

        is_zero = true;
        U8 index = 0;
        typename std::make_unsigned<N>::type mask = N(-1);
        for (int i = sizeof(N) + 3 - 2; i >= 0; i--) {
            // equivalent to 'mask >>= (1 << i)' but without compiler warnings
            mask = (decltype(mask)) (U64(mask) >> (1 << i));
            index <<= 1;

            if (check_equal_zero(n & mask)) { // note that we don't need to check all bits each time
                // equivalent to 'mask >>= (1 << i)' but without compiler warnings
                mask = (decltype(mask)) (U64(mask) << (1 << i));
                index |= 1;
            }
            else {
                is_zero = false;
            }
        }

        // handle the edge case for n = 0. We don't use special functions for this check since this is very easy to
        // handle in the circuit
        if (index == sizeof(N) * 8 - 1) {
            if ((n & mask) == 0) { // check for the MSB of n
                index = 0;
            }
            else {
                is_zero = false; // edge case where only the last bit of n is set
            }
        }

        return index;
    }


    template<typename N>
    constexpr U8 get_last_set_bit_index(const N n, bit& is_zero)
    {
        static_assert(std::is_integral<N>{}, "get_last_set_bit_index operand must be of integral type");

        constexpr U8 loops_count = get_binary_tree_depth<N>() - 1;
        static_assert(loops_count >= 0);

        is_zero = true;
        U8 index = 0;
        typename std::make_unsigned<N>::type mask = N(-1);
        for (int i = loops_count; i >= 0; i--) {
            // equivalent to 'mask >>= (1 << i)' but without compiler warnings
            mask = (decltype(mask)) (U64(mask) << (1 << i));
            index <<= 1;

            if (check_different_than_zero(n & mask)) { // note that we don't need to check all bits each time
                index |= 1;
                is_zero = false;
            }
            else {
                // equivalent to 'mask >>= (1 << i)' but without compiler warnings
                mask = (decltype(mask)) (U64(mask) >> (1 << i));
            }
        }

        // handle the edge case for n = 1. We don't use special functions for this check since this is very easy to
        // handle in the circuit
        if (index == 0) {
            if ((n & 0b1) == 1) {
                is_zero = false;
            }
        }

        return index;
    }


    template<typename N>
    constexpr U8 get_last_set_bit_index_no_zero(const N n)
    {
        bit _ = 0;
        return get_last_set_bit_index(n, _);
    }


    template<typename N>
    constexpr bit get_and_set_bit_at(N& n, const U8 pos, const bit val, const bool _no_set = false)
    {
        static_assert(std::is_integral<N>{}, "get_and_set_bit_at operand must be of integral type");
        static_assert(sizeof(N) <= sizeof(U32), "get_and_set_bit_at operand must be a 32 bit type or smaller");

        // This is implemented as a binary tree in the circuit.
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
                default: break; // shouldn't happen
                }
            }
            else {
                switch (i) {
                case 0: bit_mask &= 0x0001 << offset; break;
                case 1: bit_mask &= 0x0003 << offset; break;
                case 2: bit_mask &= 0x000F << offset; break;
                case 3: bit_mask &= 0x00FF << offset; break;
                case 4: bit_mask &= 0xFFFF << offset; break;
                default: break; // shouldn't happen
                }
            }
        }

        bit is_bit_set = bit(n & bit_mask);

        if (!_no_set) {
            n &= ~bit_mask;
            if (val) {
                n |= bit_mask;
            }
        }

        return is_bit_set;
    }


    template<typename N>
    constexpr bit get_bit_at(const N n, const U8 pos)
    {
        // 'get_bit_at' and 'get_and_set_bit_at' share the same circuit,
        // because they are very similar, and not much used.
        N n_ = n;
        return get_and_set_bit_at(n_, pos, 0, true);
    }


    /**
     * Addition is implemented so that the sign of the operands doesn't matter.
     * This means that the same algorithm can be used for subtraction if b < 0.
     *
     * The algorithm used is the same as the usual pen and paper addition.
     */
    template<typename A, typename B>
    constexpr A add(const A a, const B b, bit& carry)
    {
        static_assert(std::is_integral<A>{} && std::is_integral<B>{}, "Add operands must be of integral type");
        static_assert(sizeof(A) >= sizeof(B), "First operand of 'add' must have at least the same bit length than the second");

        A stack = 0;
        typename std::make_unsigned<A>::type mask = 1, tmp;
        typename std::make_unsigned<A>::type _carry = bit(carry);
        for (int i = 0; i < sizeof(A) * 8; i++) {
            if (i != 0) {
                _carry <<= 1;
            }
            tmp = (a & mask) ^ (b & mask);
            stack |= tmp ^ _carry;
            _carry = (a & b & mask) | (tmp & _carry);
            mask <<= 1;
        }

        carry = bit(_carry);
        return stack;
    }


    template<typename A, typename B>
    constexpr A add_no_carry(const A a, const B b)
    {
        bit carry = 0;
        return add(a, b, carry);
    }


    /**
     * @brief the two's complement of the input.
     * This can be interpreted as negating it.
     */
    template<typename N>
    constexpr N negate(const N n)
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


    template<typename A, typename B>
    constexpr A sub(const A a, const B b, bit& carry)
    {
        B b_ = ALU::negate(b);
        return ALU::add(a, b_, carry);
    }


    template<typename A, typename B>
    constexpr A sub_no_carry(const A a, const B b)
    {
        bit carry = 0;
        B b_ = ALU::negate(b);
        return ALU::add(a, b_, carry);
    }


    template<typename N>
    constexpr N abs(const N n)
    {
        static_assert(std::is_integral<N>{}, "abs operand must be of integral type");

        N res;
        if (check_is_negative(n, OpSize::UNKNOWN)) {
            res = negate(n);
        }
        else {
            res = n;
        }

        return res;
    }


    /**
     * Pen and paper multiplication.
     * Minimises the number of additions performed.
     * Shifts are trivial in the circuit implementation, since the loop is unrolled bit lines are shifted in the circuit.
     *
     * TODO : maybe make a signed version in order to optimize multiplication with small negative numbers, where numbers
     *  are made positive, then multiplied, then the sign is added back
     */
    template<typename A, typename B>
    constexpr A multiply(const A a, const B b, bit& overflow)
    {
        static_assert(std::is_integral<A>{} && std::is_integral<B>{}, "Multiply operands must be of integral type");
        static_assert(sizeof(A) >= sizeof(B), "First operand of 'multiply' must have at least the same bit length than the second");

        // cast to unsigned with the same bit length for all operands
        typename std::make_unsigned<A>::type a_bits = a;
        typename std::make_unsigned<A>::type b_bits = b;

        bit carry;
        typename std::make_unsigned<A>::type stack_bits = 0;
        for (int i = 0; i < sizeof(A) * 8; i++) {
            if (b_bits & 1) {
                carry = 0; // do not use the carry for the addition, it is only here for the overflow flag
                stack_bits = add(stack_bits, a_bits, carry); // stack += a
                overflow |= carry;
            }
            a_bits <<= 1;
            b_bits >>= 1;
        }

        return stack_bits; // will cast to signed if necessary
    }


    template<typename A, typename B>
    constexpr A multiply_no_overflow(const A a, const B b)
    {
        bit overflow = 0;
        return multiply(a, b, overflow);
    }


    /**
     * Pen and paper division.
     * If the divisor is zero, there is no division and the quotient and remainder are zero.
     */
    template<typename N, typename D>
    constexpr void unsigned_divide(const N n, const D d, N& q, N& r, bit& divByZero)
    {
        static_assert(std::is_integral<N>{} && std::is_integral<D>{}, "Division operands must be of integral type");
        static_assert(std::is_unsigned<N>{} && std::is_unsigned<D>{}, "Unsigned Division operands must be unsigned");
        static_assert(sizeof(N) >= sizeof(D), "Dividend must have at least the same bit length of the divisor");

        N _d = d; // make sure to have enough space for shifting later with this cast
        q = 0;

        // This initialisation part searches for the minimum shift needed for an efficient division.
        // Because we use registers with the same size as the operands, we must also do this for the divisor,
        // because if we don't we might subtract n with only a fraction of d.
        // This is equivalent to:
        /*
        U8 min_shift_num = get_last_set_bit_index(n) - 1;
        U8 min_shift_denominator = sizeof(N) * 8 - get_last_set_bit_index(_d);
        U8 min_shift = min(min_shift_num, min_shift_denominator)
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

        U8 min_shift;
        if (compare_greater_or_equal(min_n, min_d)) {
            min_shift = min_d;
        }
        else {
            min_shift = min_n;
        }

        // Division algorithm. Once again this is the same one as the pen and paper version, but in binary.
        // We start the computation once we shifted d enough, meaning that all of its bits are available and
        // we reached (or went farther than) the most significant set bit of the numerator. This greatly
        // reduces the number of operations, as well as making it correct, since in the circuit implementation
        // the loop is unrolled. Divisions like 1 / 2^31 costs as much as 2 / 2.
        // The computation is also ended when we reach a point where n == (d << i), since n = 0 and there is
        // nothing left to do.
        // Note that here we use 'd << i' for the value of d shifted 'nb bits - i' times to the left, to
        // preserve all bits of d. In the actual circuit d is shifted to the right once per step.
        N stack = n;
        bit can_compute = false;
        bit equal;
        for (int i = (sizeof(N) * 8 - 1); i >= 0; i--) {
            if (min_shift == i) {
                can_compute = true;
                _d <<= i;
            }

            q <<= 1;

            if (can_compute) {
                if (compare_greater_or_equal_with_eq(stack, _d, equal)) {
                    stack = add_no_carry(stack, negate(_d)); // n -= d
                    q |= 1;

                    // ignore those warnings they lie
                    if (equal) { // n == d
                        can_compute = false; // equivalent to break
                    }
                }
                _d >>= 1;
            }
        }

        r = stack;
    }


    template<typename N, typename D>
    constexpr void signed_divide(const N n, const D d, N& q, N& r, bit& divByZero)
    {
        static_assert(std::is_integral<N>{} && std::is_integral<D>{}, "Division operands must be of integral type");

        typename std::make_unsigned<N>::type n_unsigned = abs(n);
        typename std::make_unsigned<D>::type d_unsigned = abs(d);

        typename std::make_unsigned<N>::type q_unsigned = 0, r_unsigned = 0;

        unsigned_divide(n_unsigned, d_unsigned, q_unsigned, r_unsigned, divByZero);

        if (divByZero) {
            return;
        }

        q = q_unsigned;
        r = r_unsigned;

        bit sign = (check_is_negative(n, OpSize::UNKNOWN)) ^(check_is_negative(d, OpSize::UNKNOWN));
        if (sign) {
            // the result is negative
            q = negate(q);
            if (check_different_than_zero(r)) {
                r = add_no_carry(d, negate(r)); // reverse the remainder (r = d - r)
            }
        }
    }
}
