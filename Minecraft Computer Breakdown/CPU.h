#pragma once

#include <exception>

#include "data_types.h"
#include "ALU.hpp"
#include "instructions.hpp"
#include "registers.h"
#include "RAM.h"
#include "ROM.h"


class BadInstruction : public std::exception
{
	const char* msg;

public:
	BadInstruction() : msg("") {}

	BadInstruction(const char* msg, const int pos) noexcept
	{
		const int buffer_size = strlen(msg) + 20;
		char* buffer = new char[];
		snprintf(buffer, buffer_size, "'%s' at %d", msg, pos);
		this->msg = buffer;
	}

	~BadInstruction() { delete msg; }

	virtual const char* what() const noexcept { return msg; }
};


class StopInstruction : public std::exception
{
	const char* msg;

public:
	StopInstruction(const int pos) noexcept
	{
		char buffer[50];
		snprintf(buffer, 50, "Stop instruction reached at %d", pos);
		this->msg = buffer;
	}

	~StopInstruction() { delete msg; }

	virtual const char* what() const noexcept { return msg; }
};


class CPU
{
	Registers registers;
	RAM<512> ram;
	ROM<U32, 512> rom;

	const Inst** instructions;
	const U32 instructions_count;

	void update_overflow_flag(U32 op1, U32 op2, U32 result);
	void update_sign_flag(U32 result);
	void update_zero_flag(U32 result);
	void update_adjust_flag(U32 op1, U32 op2, U8 register_);
	void update_parity_flag(U32 result);
	void update_carry_flag(bit carry);
	void update_status_flags(U32 op1, U32 op2, U32 result, U8 register_, bit carry = 0);

public:
	CPU(const Inst** instructions, const U32 count) : instructions(instructions), instructions_count(count) {}

	~CPU()
	{
		for (int i = 0; i < instructions_count; i++) {
			delete instructions[i];
		}
		delete[] instructions;
	}

	void run();
	void execute_instruction();
};
