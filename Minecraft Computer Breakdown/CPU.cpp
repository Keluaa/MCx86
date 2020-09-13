
#include <iostream>

#include "CPU.h"

#define NYI throw NotImplemented(opcode, registers.EIP)

CPU::CPU(const Inst** instructions, const U32 count)
	: instructions(instructions), instructions_count(count) 
{
	switch_protected_mode(); // use protected mode by default
}

void CPU::switch_protected_mode(bit protected_)
{
	registers.control_flag_write_PE(protected_);
}

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
		catch (ExceptionWithMsg& e) {
			std::cerr << e.what() << "\n";
			break;
		}
	}

	std::cout << "Program finished in " << cycle << " cycles.\n";
}

void CPU::execute_instruction()
{
	const Inst* inst = instructions[registers.EIP];

	ALU::branchMonitor.reset();

	// fetch the value of the first operand
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

	// fetch the value of the second operand
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
	U16 opcode = inst->two_bytes_opcode ? (0x0F00 + inst->opcode) : inst->opcode;
	switch (opcode)
	{
	case ISA::Opcodes::AAA:
	{
		U16 AX_val = registers.read(ISA::Registers::AX);
		// if AF is set or if the first 4 bits of AL are greater than 9 
		if (registers.flag_read_AF() ||
				ALU::compare_greater_or_equal(AX_val & 0x0F, 10)) {
			// we do
			// AL <- (AL + 6) and 0x0F
			// AH <- AH + 1
			// but in one step:
			// AX <- (AH + 1) : (AL + 6) & 0x0F
			// or:
			// AX <- (AX + 0x0106) & 0x0F0F <- 0F0F mask because we are doing BCD operations
			// this is because the adder can only be used once per clock cycle
			registers.write(ISA::Registers::AX, ALU::add_no_carry(AX_val, (U16) 0x0106) & 0x0F0F);
			
			registers.flag_write_AF(1);
			registers.flag_write_CF(1);
		}
		else {
			registers.flag_write_AF(0);
			registers.flag_write_CF(0);
		}
		break;
	}
	case ISA::Opcodes::AAD:
	{
		U8 AH_val = registers.read(ISA::Registers::AH);
		U8 AL_val = registers.read(ISA::Registers::AL);

		AL_val = ALU::add_no_carry(ALU::multiply(AH_val, (U8)10), AL_val);

		registers.write(ISA::Registers::AL, AL_val);
		registers.write(ISA::Registers::AH, 0);

		update_sign_flag(AL_val);
		update_zero_flag(AL_val);
		update_parity_flag(AL_val);
		break;
	}
	case ISA::Opcodes::AAM:
	{
		U8 q, r;
		bit divByZero;
		ALU::unsigned_divide((U8) registers.read(ISA::Registers::AL), (U8)10, q, r, divByZero);
		
		registers.write(ISA::Registers::AH, q);
		registers.write(ISA::Registers::AL, r);

		update_sign_flag(r);
		update_zero_flag(r);
		update_parity_flag(r);
		break;
	}
	case ISA::Opcodes::AAS:
	{
		U16 AX_val = registers.read(ISA::Registers::AX);
		// if AF is set or if the first 4 bits of AL are greater than 9 
		if (registers.flag_read_AF() ||
				ALU::compare_greater_or_equal(AX_val & 0x0F, 10)) {
			// we do:
			// AL <- (AL - 6) & 0x0F
			// AH <- AH - 1
			// but in one step:
			// AX <- (AX + ((-1 << 8) + (-6 & 0x0F))) & 0xFF0F
			// notice: -6 & 0x0F = 0x0A
			// and then: (-1 << 8) + (-6 & 0x0F) = 0xFF0A
			// This is for the same reason as for the AAA instruction
			registers.write(ISA::Registers::AX, ALU::add_no_carry(AX_val, (U16)0xFF0A) & 0xFF0F);

			registers.flag_write_AF(1);
			registers.flag_write_CF(1);
		}
		else {
			registers.flag_write_AF(0);
			registers.flag_write_CF(0);
		}
		break;
	}
	case ISA::Opcodes::ADC:
	{
		bit carry = registers.flag_read_CF();
		op1_val_out = ALU::add(op1_val, op2_val, carry);
		op1_val_out_set = 1;

		update_status_flags(op1_val, op2_val, op1_val_out, (inst->op1_type == Inst::R ? inst->op1 : -1), carry);
		break;
	}
	case ISA::Opcodes::ADD:
	{
		bit carry = 0;
		op1_val_out = ALU::add(op1_val, op2_val, carry);
		op1_val_out_set = 1;
		
		update_status_flags(op1_val, op2_val, op1_val_out, (inst->op1_type == Inst::R ? inst->op1 : -1), carry);
		break;
	}
	case ISA::Opcodes::AND:
	{
		op1_val_out = ALU::and_(op1_val, op2_val);
		op1_val_out_set = 1;
		
		registers.flag_write_CF(0);
		registers.flag_write_OF(0);
		update_parity_flag(op1_val_out);
		update_sign_flag(op1_val_out);
		update_zero_flag(op1_val_out);
		break;
	}
	case ISA::Opcodes::ARPL:
	{
		// compare the first two bits of the operands, and branch if op1 < op2
		if (!ALU::compare_greater_or_equal(op1_val & 0x03, op2_val & 0x03)) {
			// copy the first two bits of op2 to the first two bits of op1
			op1_val_out = (op1_val & (0xFF - 0x03)) | (op2_val & 0x03);
			op1_val_out_set = 1;

			registers.flag_write_ZF(1);
		}
		else {
			registers.flag_write_ZF(0);
		}
		break;
	}
	case ISA::Opcodes::BOUND:
	{
		NYI;
	}
	case ISA::Opcodes::BSF:
	{
		bit isZero;
		op1_val_out = ALU::get_first_set_bit_index(op1_val, isZero);
		op1_val_out_set = 1;

		registers.flag_write_ZF(isZero);
		break;
	}
	case ISA::Opcodes::BSR:
	{
		bit isZero;
		op1_val_out = ALU::get_last_set_bit_index(op1_val, isZero);
		op1_val_out_set = 1;

		registers.flag_write_ZF(isZero);
		break;
	}
	case ISA::Opcodes::BT:
	{
		if (inst->op1_type == Inst::M) {
			WARNING("The BT instruction has an incomplete implementation for memomry operands.")
		}
		
		bit bit_val = ALU::get_bit_at(op1_val, op2_val);
		registers.flag_write_CF(bit_val);
		break;
	}
	case ISA::Opcodes::BTC:
	{
		if (inst->op1_type == Inst::M) {
			WARNING("The BTC instruction has an incomplete implementation for memomry operands.")
		}

		bit bit_val = ALU::get_bit_at(op1_val, op2_val);
		registers.flag_write_CF(!bit_val);
		break;
	}
	case ISA::Opcodes::BTR:
	{
		if (inst->op1_type == Inst::M) {
			WARNING("The BTR instruction has an incomplete implementation for memomry operands.")
		}

		bit bit_val = ALU::get_and_set_bit_at(op1_val, op2_val, 0);
		op1_val_out = op1_val;
		op1_val_out_set = 1;
		registers.flag_write_CF(bit_val);
		break;
	}
	case ISA::Opcodes::BTS:
	{
		if (inst->op1_type == Inst::M) {
			WARNING("The BTS instruction has an incomplete implementation for memomry operands.")
		}

		bit bit_val = ALU::get_and_set_bit_at(op1_val, op2_val, 1);
		op1_val_out = op1_val;
		op1_val_out_set = 1;
		registers.flag_write_CF(bit_val);
		break;
	}
	case ISA::Opcodes::CALL:
	{
		NYI;
		break;
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
		snprintf(buffer, 50, "Unknown instruction: %x", (int)opcode);
		throw BadInstruction(buffer, registers.EIP);
	}
	}
	registers.EIP++;

	if (op1_val_out_set) {
		// write the new value back to its register / memory address
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

void CPU::push_2(U16 value)
{

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
