#pragma once

#include <stack>
#include <limits>

#include "../data_types.h"
#include "instructions.h"
#include "registers.h"
#include "memory/memory_manager.hpp"
#include "memory/RAM.hpp"
#include "memory/ROM.hpp"
#include "memory/stack.hpp"
#include "memory/buffer.hpp"
#include "exceptions.h"
#include "interrupts.h"


class CPU
{
	Registers registers;

    Mem::Memory* const memory;

    Mem::Buffer<128, U8> io = Mem::Buffer<128, U8>(new U8[128]{});

	Interrupts::InterruptDescriptorTable<64>* interrupts_table = nullptr; // TODO

	const Inst* current_instruction = nullptr;

	U32 clock_cycle_count = 0;
    bit halted = false;

	[[nodiscard]] static constexpr OpSize get_size(bit size_override, bit byte_size_override);
	
	void execute_arithmetic_instruction(U8 opcode, const InstData data, EFLAGS& flags, U32& ret, U32& ret_2);
	void execute_non_arithmetic_instruction(const U8 opcode, const InstData data, EFLAGS& flags, U32& ret, U32& ret_2);
	void execute_non_arithmetic_instruction_with_state_machine(const U8 opcode, const InstData data, EFLAGS& flags, U32& ret);
	
	[[nodiscard]] U32 compute_address(U8 register_field) const;
	
	void push(U32 value, OpSize size = OpSize::UNKNOWN);
	U32 pop(OpSize size = OpSize::UNKNOWN);

	void interrupt(Interrupts::Interrupt interrupt, U8 error_code = 0);

    void throw_NYI(const char* msg) const
    {
        throw NotImplemented(current_instruction->opcode, registers.EIP, msg);
    }

    void throw_exception(const Interrupts::Interrupt& interrupt) const
    {
        throw ProcessorException(interrupt.mnemonic, registers.EIP, interrupt.vector);
    }

public:
    explicit CPU(Mem::Memory* memory);

	~CPU()
	{
        delete io.get_bytes();
	}

	void new_clock_cycle();
	void startup();
	void run(size_t max_cycles = std::numeric_limits<size_t>::max());
	void execute_instruction();

	Registers& get_registers() { return registers; }
	Mem::Memory& get_memory() { return *memory; }

    U32 read_io(U8 io_address, OpSize size);
    void write_io(U8 io_address, U32 value, OpSize size);

	[[nodiscard]]
	bool is_halted() const { return halted; }

	[[nodiscard]]
	U32 get_clock_cycle() const { return clock_cycle_count; }
};
