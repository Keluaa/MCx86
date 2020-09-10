
#include <iostream>

#include "CPU.h"

void CPU::run()
{
	const int max_cycles = 1000;
	int cycle = 0;
	while (registers.EIP < instructions_count) {
		cycle++;
		if (cycle >= max_cycles) {
			std::cout << "Max cycles reached. Interrupting program.\n";
			break;
		}

		try {
			execute_instruction(); // increments the EIP register
		}
		catch (BadInstruction& badInst) {
			std::cerr << badInst.what() << "\n";
			break;
		}
		catch (StopInstruction& stop) {
			std::cout << stop.what() << "\n";
			break;
		}
	}

	std::cout << "Program finished in " << cycle << " cycles.\n";
}

void CPU::execute_instruction()
{
	const Inst* inst = instructions[registers.EIP];

	U32 op1_val = 0;
	switch (inst->op1_type)
	{
	case Inst::I: // Immediate
		op1_val = inst->op1;
		break;

	case Inst::R: // Register
		op1_val = registers.read(inst->op1);
		break;

	case Inst::M: // Memory
		op1_val = ram.read(inst->op1);
		break;

	case Inst::None:
		break;
	}

	U32 op2_val = 0;
	switch (inst->op2_type)
	{
	case Inst::I: // Immediate
		op2_val = inst->op2;
		break;

	case Inst::R: // Register
		op2_val = registers.read(inst->op2);
		break;

	case Inst::M: // Memory
		op2_val = ram.read(inst->op2);
		break;

	case Inst::None:
		break;
	}

	bit op1_val_out_set = 0;
	U32 op1_val_out = 0;

	// perform the instruction

	switch (inst->opcode)
	{
	case ISA::Opcodes::ADD:
	{
		bit carry = 0;
		op1_val_out = ALU::add(op1_val, op2_val, carry);
		op1_val_out_set = 1;
		update_status_flags(op1_val, op2_val, op1_val_out, (inst->op1_type == Inst::R ? inst->op1 : -1), carry);
	}
	case ISA::Opcodes::MOV:
	{
		op1_val_out = op2_val;
		op1_val_out_set = 1;
		break;
	}
	case ISA::Opcodes::JO:
	{
		if (registers.flag_read_OF()) {
			registers.EIP = op1_val;
			return; // skip EIP increment
		}
		break;
	}
	case ISA::Opcodes::JNO:
	{
		if (!registers.flag_read_OF()) {
			registers.EIP = op1_val;
			return; // skip EIP increment
		}
		break;
	}
	case ISA::Opcodes::XCHG:
	{
		// op1 <-op2
		switch (inst->op1_type) {
		case Inst::R:
			registers.write(inst->op1, op2_val);
			break;
		case Inst::M:
			ram.write(inst->op1, op2_val);
			break;
		default:
			throw BadInstruction("Wrong type of operand for XCHG", registers.EIP);
		}
		// op2 <- op1
		switch (inst->op2_type) {
		case Inst::R:
			registers.write(inst->op2, op1_val);
			break;
		case Inst::M:
			ram.write(inst->op2, op1_val);
			break;
		default:
			throw BadInstruction("Wrong type of operand for XCHG", registers.EIP);
		}
		break;
	}
	case ISA::Opcodes::MUL:
	{
		throw BadInstruction("MUL instruction should not be used. Use IMUL instead.", registers.EIP);
	}
	case ISA::Opcodes::IMUL:
	{
		// supports only the 2 operand form. This implies that the result is always truncated to
		// match the size of the destination. This is made on purpose because it greatly simplifies
		// the circuit implementation: no need for a double size output (64bits max), to write to
		// serveral registers at once or to handle 3 operands at once. But most importantly, this
		// means that the multiplier is 4 times smaller (half of the bits to be handled half of the 
		// times). 
		// Because of those benefits, there is no plan to support extended multiplication.
		// The 3 operands form behaves like the 2 operands one, but with an immediate value, it will
		// maybe be implemented.
		if (inst->op2_type == Inst::None) {
			throw BadInstruction("Only the 2 operands version of IMUL is supported.", registers.EIP);
		}
		op1_val_out = ALU::multiply(op1_val, op2_val);
		op1_val_out_set = 1;

		bit expected_sign = (ALU::check_is_negative(op1_val)) ^ (ALU::check_is_negative(op2_val));
		bit overflow = expected_sign ^ ALU::check_is_negative(op1_val_out);
		registers.flag_write_CF(overflow);
		registers.flag_write_OF(overflow);
		break;
	}
	case ISA::Opcodes::STOP:
	{
		throw StopInstruction(registers.EIP);
	}
	default:
	{
		char buffer[50];
		snprintf(buffer, 50, "Unknown instruction: %x", (int)inst->opcode);
		throw BadInstruction(buffer, registers.EIP);
	}
	}
	registers.EIP++;

	// write new values back to registers / memory

	if (op1_val_out_set) {
		switch (inst->op1_type)
		{
		case Inst::R: // Register
			registers.write(inst->op1, op1_val_out);
			break;

		case Inst::M: // Memory
			ram.write(inst->op1, op1_val_out);
			break;

		case Inst::I: // Immediate
		case Inst::None:
			break;
		}
	}
}

void CPU::update_overflow_flag(U32 op1, U32 op2, U32 result)
{
	/*
	overflow flag:
	1 2 R OF
	+ + + 0
	+ + - 1
	+ - / 0
	- + / 0
	- - - 0
	- - + 1

	=> (s(op1) & s(op2)) ^ s(R)
	*/
	registers.flag_write_OF((ALU::check_is_negative(op1) & ALU::check_is_negative(op2)) ^ ALU::check_is_negative(result));
}

void CPU::update_sign_flag(U32 result)
{
	registers.flag_write_SF(ALU::check_is_negative(result));
}

void CPU::update_zero_flag(U32 result)
{
	registers.flag_write_ZF(ALU::check_equal_zero(result));
}

void CPU::update_adjust_flag(U32 op1, U32 op2, U8 register_)
{
	/*
	Adjust flag is set only if there were an carry from the first 4 bits of the AL register to the 4 other bits.
	It is 0 otherwise, including when the operation didn't used the AL register.
	This function should only be called with instructions modifing the AL register (or AX and EAX, but not AH).
	*/
	switch (register_)
	{
	case ISA::Registers::AL:
	case ISA::Registers::AX:
	case ISA::Registers::EAX:
		// Not the implementation used in the circuit, which is much simpler as this flag comes out from the adder
		// directly.
		registers.flag_write_AF((op1 & 0x0F) + (op2 & 0x0F) > 0x0F);
		break;

	default:
		registers.flag_write_AF(0);
		break;
	}
}

void CPU::update_parity_flag(U32 result)
{
	/*
	Parity check is made only on the first byte
	*/
	registers.flag_write_PF(ALU::check_parity(U8(result)));
}

void CPU::update_carry_flag(bit carry)
{
	registers.flag_write_CF(carry);
}

void CPU::update_status_flags(U32 op1, U32 op2, U32 result, U8 register_, bit carry)
{
	// updates all status flags
	update_overflow_flag(op1, op2, result);
	update_sign_flag(result);
	update_zero_flag(result);
	update_adjust_flag(op1, op2, register_);
	update_parity_flag(result);
	update_carry_flag(carry);
}
