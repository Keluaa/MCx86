#pragma once

#include <cstring>
#include <cstdio>
#include <exception>
#include <memory>

#include "data_types.h"


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
	BadInstruction(const char* msg, const int pos) noexcept
	{
		const size_t buffer_size = strlen(msg) + 20;
		char* buffer = new char[buffer_size];
		snprintf(buffer, buffer_size, "'%s' at %d", msg, pos);
		this->msg = buffer;
	}
};


class NotImplemented : public ExceptionWithMsg
{
public:
	NotImplemented(const U16 opcode, const int pos, const char* msg) noexcept
	{
	    const size_t buffer_size = strlen(msg) + 50;
		char* buffer = new char[buffer_size];
		snprintf(buffer, buffer_size, "Instruction %x at %d not implemented: %s", (int) opcode, pos, msg);
		this->msg = buffer;
	}
};


class UnknownInstruction : public ExceptionWithMsg
{
public:
	UnknownInstruction(const char* msg, const U16 opcode, const int pos)
	{
		const size_t buffer_size = strlen(msg) + 20;
		char* buffer = new char[buffer_size];
		snprintf(buffer, buffer_size, "%s (opcode: %x, at %d)", msg, (int)opcode, pos);
		this->msg = buffer;
	}
};


class ProcessorException : public ExceptionWithMsg
{
public:
	ProcessorException(const char* mnemonic, const int pos, U8 code = -1)
	{
		const size_t buffer_size = strlen(mnemonic) + 30;
		char* buffer = new char[buffer_size];
		snprintf(buffer, buffer_size, "Exception %s(%d) at %d", mnemonic, code, pos);
		this->msg = buffer;
	}
};


inline void print_warning(const char* msg, const char* function, int position, const char* file)
{
	static bool arg = true;
	fprintf(stderr, "Warning in file %s at line %d in function '%s': %s\n", file, position, function, msg);
}


#define _WARNING_EVAL_2(msg, file, line, func) { static bool $____warning_printed_flag_ ## line = false; if (!$____warning_printed_flag_ ## line) { print_warning(msg, func, line, file); $____warning_printed_flag_ ## line = true; } }
#define _WARNING_EVAL_1(msg, file, line, func) _WARNING_EVAL_2(msg, file, line, func)
#define WARNING(msg) _WARNING_EVAL_1(msg, __ ## FILE__, __ ## LINE__, __ ## func__) false
