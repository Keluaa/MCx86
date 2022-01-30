
#include "doctest.h"

#include "ALU.hpp"


TEST_SUITE("ALU_negate")
{
    TEST_CASE("negate_positive_1")
    {
        I32 n = 1;
        I32 r = ALU::negate(n);

        REQUIRE(-n == r);
    }

    TEST_CASE("negate_negative_1")
    {
        U32 n = -1;
        U32 r = ALU::negate(n);

        REQUIRE(U32(-I32(n)) == r);
    }

    TEST_CASE("negate_any")
    {
        I16 n = 32;
        I16 r = ALU::negate(n);

        REQUIRE(-n == r);
    }
}


TEST_SUITE("ALU_add")
{
    TEST_CASE("add_positive")
    {
        U32 a = 1;
        U16 b = 42;
        bit carry = 0;
        U32 r = ALU::add(a, b, carry);

        REQUIRE_EQ(a + b, r);
        REQUIRE(carry == 0);
    }


    TEST_CASE("add_negative")
    {
        U32 a = 1;
        I16 b = -42;
        bit carry = 0;
        U32 r = ALU::add(a, b, carry);

        REQUIRE_EQ(a + b, r);
        REQUIRE(carry == 0);
    }

    TEST_CASE("add_overflow")
    {
        U32 a = 1;
        U32 b = -1;
        bit carry = 0;
        U32 r = ALU::add(a, b, carry);

        REQUIRE(r == 0);
        REQUIRE(carry == 1);
    }

    TEST_CASE("add_with_carry")
    {
        U32 a = 1;
        U32 b = 1;
        bit carry = 1;
        U32 r = ALU::add(a, b, carry);

        REQUIRE_EQ(a + b + 1, r);
        REQUIRE(carry == 0);
    }
}


TEST_SUITE("ALU_multiply")
{
    TEST_CASE("multiply_positive")
    {
        bit overflow = 0;
        U16 a = 37;
        U16 b = 12;
        U16 r = ALU::multiply(a, b, overflow);

        REQUIRE(a * b == r);
        REQUIRE(!overflow);
    }

    TEST_CASE("multiply_negative")
    {
        bit overflow = 0;
        I16 a = -45;
        I16 b = 11;
        I16 r = ALU::multiply(a, b, overflow);

        REQUIRE(a * b == r);
        REQUIRE(overflow); // the overflow flag should be set as the MSB are set for negative integers
    }

    TEST_CASE("multiply_overflow")
    {
        bit overflow = 0;
        I16 a = 25648;
        U16 b = 54621;
        I16 r = ALU::multiply(a, b, overflow);

        REQUIRE(I16(a * b) == r);
        REQUIRE(overflow);
    }
}


TEST_SUITE("ALU_divide")
{
    TEST_CASE("divide_positive")
    {
        U32 n = 64;
        U16 d = 16;
        U32 q, r;
        bit divByZero;
        ALU::unsigned_divide(n, d, q, r, divByZero);

        REQUIRE(q == (n / d));
        REQUIRE(r == (n % d));
        REQUIRE(!divByZero);
    }

    TEST_CASE("divide_by_zero")
    {
        U32 n = 61;
        U16 d = 0;
        U32 q, r;
        bit divByZero;
        ALU::unsigned_divide(n, d, q, r, divByZero);

        REQUIRE(divByZero);
    }
}


TEST_SUITE("ALU_get_bit_at")
{
    TEST_CASE("get_bit_at")
    {
        U32 n = 1;
        for (int i = 0; i < sizeof(U32) * 8; i++) {
            for (int j = 0; j < sizeof(U32) * 8; j++) {
                bit i_bit = ALU::get_bit_at(n, j);

                REQUIRE_EQ(i_bit, i == j);
            }

            n <<= 1;
        }
    }

    TEST_CASE("get_and_set_bit_at")
    {
        U32 n = 1 << 9;
        bit i_bit = ALU::get_and_set_bit_at(n, 9, 0);

        REQUIRE(i_bit);
        REQUIRE(n == 0);
    }
}


TEST_SUITE("ALU_get_last_or_first_set_bit")
{
    TEST_CASE("get_last_set_bit_zero")
    {
        bit is_zero;
        U16 n = 0;
        U8 index = ALU::get_last_set_bit_index(n, is_zero);
        REQUIRE(index == 0);
        REQUIRE(is_zero);
    }

    TEST_CASE("get_last_set_bit")
    {
        bit is_zero;
        U16 n;
        U8 index;

        for (int i = 0; i < sizeof(n) * 8; i++) {
            n = 1 << i;
            CAPTURE(i);
            index = ALU::get_last_set_bit_index(n, is_zero);
            REQUIRE(index == i);
            REQUIRE(!is_zero);
        }
    }

    TEST_CASE("get_last_set_bit_U64")
    {
        bit is_zero;
        U64 n;
        U8 index;

        for (int i = 0; i < sizeof(n) * 8; i++) {
            n = U64(1) << i;
            CAPTURE(i);
            index = ALU::get_last_set_bit_index(n, is_zero);
            REQUIRE(index == i);
            REQUIRE(!is_zero);
        }
    }

    TEST_CASE("get_first_set_bit_zero")
    {
        bit is_zero;
        U16 n = 0;
        U8 index = ALU::get_first_set_bit_index(n, is_zero);
        REQUIRE(index == 0);
        REQUIRE(is_zero);
    }

    TEST_CASE("get_first_set_bit")
    {
        bit is_zero;
        U16 n;
        U8 index;

        for (int i = 0; i < sizeof(n) * 8; i++) {
            n = 1 << i;
            CAPTURE(i);
            index = ALU::get_first_set_bit_index(n, is_zero);
            REQUIRE(index == i);
            REQUIRE(!is_zero);
        }
    }

    TEST_CASE("get_first_set_bit_U64")
    {
        bit is_zero;
        U64 n;
        U8 index;

        for (int i = 0; i < sizeof(n) * 8; i++) {
            n = U64(1) << i;
            CAPTURE(i);
            index = ALU::get_first_set_bit_index(n, is_zero);
            REQUIRE(index == i);
            REQUIRE(!is_zero);
        }
    }
}


TEST_SUITE("ALU_equal")
{
    TEST_CASE("equal_0")
    {
        U8 a = 0;
        U8 b = 0;

        bit res = ALU::compare_equal(a, b);

        REQUIRE(res == (a == b));
        
        b = 1;
        res = ALU::compare_equal(a, b);
        
        REQUIRE(res == (a == b));

        b = 0b10;
        res = ALU::compare_equal(a, b);

        REQUIRE(res == (a == b));
    }
}
