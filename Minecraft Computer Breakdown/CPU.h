#pragma once

#include "data_types.h"
#include "ALU.hpp"
#include "instructions.hpp"
#include "registers.h"
#include "RAM.h"
#include "ROM.h"
#include "exceptions.h"


class CPU
{
	Registers registers;
	RAM<512> ram;
	ROM<U32, 512> rom;

	const Inst** instructions;
	const U32 instructions_count;

	void push_2(U16 value);
	void push_4(U32 value);

	void update_overflow_flag(U32 op1, U32 op2, U32 result);
	void update_sign_flag(U32 result);
	void update_zero_flag(U32 result);
	void update_adjust_flag(U32 op1, U32 op2, U8 register_);
	void update_parity_flag(U32 result);
	void update_carry_flag(bit carry);

	void update_status_flags(U32 op1, U32 op2, U32 result, U8 register_, bit carry = 0);

public:
	CPU(const Inst** instructions, const U32 count);

	~CPU()
	{
		for (U32 i = 0; i < instructions_count; i++) {
			delete instructions[i];
		}
		delete[] instructions;
	}

	void switch_protected_mode(bit protected_ = true);

	void run();
	void execute_instruction();
};
