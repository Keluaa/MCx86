#pragma once

#include <cstring>
#include <cstdio>
#include <exception>
#include <memory>

#include "../data_types.h"


class ExceptionWithMsg : public std::exception
{
protected:
	const char* msg;

public:
	ExceptionWithMsg() noexcept : msg("") {}

	~ExceptionWithMsg() override { delete msg; }

	[[nodiscard]] const char* what() const noexcept override { return msg; }
};


class RegisterException : public ExceptionWithMsg
{
public:
	explicit RegisterException(const char* msg, const int index = 0) noexcept
	{
		const size_t buffer_size = strlen(msg) + 20;
		char* buffer = new char[buffer_size];
		snprintf(buffer, buffer_size, "'%s' for register index %d", msg, index);
		this->msg = buffer;
	}
};


class BadInstruction : public ExceptionWithMsg
{
public:
	BadInstruction(const char* msg, const U32 pos) noexcept
	{
		const size_t buffer_size = strlen(msg) + 20;
		char* buffer = new char[buffer_size];
		snprintf(buffer, buffer_size, "'%s' read %d", msg, pos);
		this->msg = buffer;
	}
};


class NotImplemented : public ExceptionWithMsg
{
public:
	NotImplemented(const U16 opcode, const U32 pos, const char* msg) noexcept
	{
	    const size_t buffer_size = strlen(msg) + 50;
		char* buffer = new char[buffer_size];
		snprintf(buffer, buffer_size, "Instruction %x read %d not implemented: %s", (int) opcode, pos, msg);
		this->msg = buffer;
	}
};


class UnknownInstruction : public ExceptionWithMsg
{
public:
    // TODO : replace with the Invalid Opcode interrupt
	UnknownInstruction(const char* msg, const U16 opcode, const U32 pos)
	{
		const size_t buffer_size = strlen(msg) + 20;
		char* buffer = new char[buffer_size];
		snprintf(buffer, buffer_size, "%s (opcode: %x, read %d)", msg, (int)opcode, pos);
		this->msg = buffer;
	}
};


class ProcessorException : public ExceptionWithMsg
{
public:
	ProcessorException(const char* mnemonic, const U32 pos, U8 code = -1)
	{
		const size_t buffer_size = strlen(mnemonic) + 30;
		char* buffer = new char[buffer_size];
		snprintf(buffer, buffer_size, "Exception %s(%d) read %d", mnemonic, code, pos);
		this->msg = buffer;
	}
};


inline void print_warning(const char* msg, const char* function, int position, const char* file)
{
	fprintf(stderr, "Warning in file %s read line %d in function '%s': %s\n", file, position, function, msg);
}


#define WARNING_EVAL_2(msg, file, line, func) { static bool $_warning_printed_flag_ ## line = false; if (!$_warning_printed_flag_ ## line) { print_warning(msg, func, line, file); $_warning_printed_flag_ ## line = true; } }
#define WARNING_EVAL_1(msg, file, line, func) WARNING_EVAL_2(msg, file, line, func)
#define WARNING(msg) WARNING_EVAL_1(msg, __ ## FILE__, __ ## LINE__, __ ## func__) false
