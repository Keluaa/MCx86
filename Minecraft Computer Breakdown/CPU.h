#pragma once

#include <stack>

#include "data_types.h"
#include "ALU.hpp"
#include "instructions.hpp"
#include "instructions_2.hpp"
#include "registers.hpp"
#include "RAM.hpp"
#include "ROM.h"
//#include "memory_manager.hpp"
#include "exceptions.h"


class CPU
{
	Registers registers;
	RAM<512> ram;
	ROM<U32, 512> rom;
	
	// stack, supposedly stored at the segment pointed by the SS register
	std::stack<U32> stack; // TODO : replace this with a low level implementation

	const Inst** instructions;
	const U32 instructions_count;
	
	U32 inst_get_operand(const Inst_2* inst, bool second = false) const;
	U32 inst_get_address(const Inst_2* inst) const;

	void write_to_register(U8 register_index, U32 value, bit operand_size_override, bit operand_byte_size_override);
	void write_to_memory(U32 address, U32 value, bit address_size_override, bit address_byte_size_override);
	
	void new_new_execute_instruction();
	void new_execute_instruction();
	
	void execute_non_arithmetic_instruction(const Inst_2* inst);
	U32 execute_arithmetic_instruction(const U8 opcode, const SizeOverrides sizeOverrides, U32& flags);
	
	bit is_32_bit_op_inst(bit op_prefix, bit D_flag_code_segment = 0) const;
	bit is_32_bit_ad_inst(bit ad_prefix, bit D_flag_code_segment = 0) const;

	void push_2(U16 value);
	void push_4(U32 value);
	
	U16 pop_2();
	U32 pop_4();

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
