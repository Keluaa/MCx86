#pragma once

#include <stack>

#include "data_types.h"
#include "ALU.hpp"
#include "instructions.h"
#include "registers.h"
#include "memory/RAM.hpp"
#include "memory/ROM.hpp"
#include "memory/stack.hpp"
#include "exceptions.h"
#include "interrupts.h"


class CPU
{
	Registers registers;
	RAM<4096, U32> ram;
	ROM<U8> rom;
    RAM<128, U8> io = RAM<128, U8>(new U8[128]{});
    Stack stack;

	Interrupts::InterruptDescriptorTable<64>* interruptsTable;

	const Inst** instructions;
	const U32 instructions_count;
	
	const Inst* currentInstruction;

	U32 clock_cycle_count = 0;

	void new_clock_cycle();
	
	[[nodiscard]] static constexpr OpSize get_size(bit size_override, bit byte_size_override, bit D_flag_code_segment = false);
	
	void execute_arithmetic_instruction(U8 opcode, const InstData data, EFLAGS& flags, U32& ret, U32& ret2);
	void execute_non_arithmetic_instruction(const U8 opcode, const InstData data, EFLAGS& flags, U32& ret, U32& ret2);
	void execute_non_arithmetic_instruction_with_state_machine(const U8 opcode, const InstData data, EFLAGS& flags);
	
	[[nodiscard]] U32 compute_address(bit _32bits_mode, OpSize opSize) const;
	
	void push(U32 value, OpSize size = OpSize::UNKNOWN);
	U32 pop(OpSize size = OpSize::UNKNOWN);

	void interrupt(Interrupts::Interrupt interrupt);

	void update_adjust_flag(EFLAGS& flags, U32 op1, U32 op2);
	void update_status_flags(EFLAGS& flags, U32 op1, U32 op2, U32 result, OpSize op1Size, OpSize op2Size, OpSize retSize, bit carry = 0);

    void throw_NYI(const char* msg) const
    {
        throw NotImplemented(currentInstruction->opcode, registers.EIP, msg);
    }

    void throw_exception(const Interrupts::Interrupt& interrupt) const
    {
        throw ProcessorException(interrupt.mnemonic, registers.EIP, interrupt.vector);
    }

public:
    CPU(Mem::Memory* memory);
	CPU(const U8* const ROM_bytes, U8* RAM_bytes, U32 stack_size, const Inst** instructions, const U32 count);

	~CPU()
	{
        delete stack.get_bytes();
        delete io.get_bytes();
	}

	void switch_protected_mode(bit protected_ = true);

	void run();
	void execute_instruction();

    U32 read_io(U8 io_address, OpSize size);
    void write_io(U8 io_address, U32 value, OpSize size);
};
