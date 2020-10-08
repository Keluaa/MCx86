﻿
#include <iostream>

#include "CPU.h"

#define NYI throw NotImplemented(opcode, registers.EIP)

#define INTERRUPT(mnemonic, code) throw ProcessorExeception(mnemonic, registers.EIP, code)

CPU::CPU(const Inst** instructions, const U32 count)
	: instructions(instructions), instructions_count(count) 
{
	// TODO: initialisation process
	//switch_protected_mode(); // use protected mode by default
}

void CPU::switch_protected_mode(bit protected_)
{
	// todo: see the startup process in the manual instead
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

OpSize CPU::get_size(bit size_override, bit byte_size_override, bit D_flag_code_segment) const
{
	if (byte_size_override) {
		return OpSize::B;
	}
	else if (size_override ^ D_flag_code_segment) {
		return OpSize::DW;
	}
	else {
		return OpSize::W;
	}
}

void CPU::execute_instruction()
{
	// TODO: I think I misunderstood what the address size override prefix: it only changes the size of the address,
	//  not how many bytes are fetched from memory, which is also controlled by the operand size prefix 
	const Inst* inst = instructions[registers.EIP]; // TODO: low level instruction fetching
	currentInstruction = inst;
	
	ALU::branchMonitor.reset();
	
	InstData data = inst->getInstData();
	
	// compute address using the mod r/m, SIB and displacement bytes
	if (inst->compute_address) {
		data.address = compute_address(!inst->address_size_override, data.op1_size); // TODO: more things to do to have the size of the address
	}
	
	// TODO: use the computed address to read the operands. Also fix the operands size.
	
	// Read the operands
	U32 op1_val = 0;
	if (inst->read_op1) {
		switch(inst->op1_type) {
		case OpType::REG:
			data.op1_size = get_size(inst->operand_size_override, inst->operand_byte_size_override);
			data.op1 = registers.read_index(inst->op1_register, data.op1_size);
			break;
		case OpType::MEM:
			data.op1_size = get_size(inst->address_size_override, inst->address_byte_size_override);
			data.op1 = ram.read(inst->address_value, data.op1_size);
			break;
		case OpType::IMM:
			data.op1_size = get_size(inst->operand_size_override, inst->operand_byte_size_override);
			data.op1 = inst->immediate_value;
			break;
		case OpType::M_M:
			// used only by BOUND for the second operand
			break;
		case OpType::SREG:
			data.op1_size = OpSize::W; // all segment registers have a fixed length
			data.op1 = registers.read_segment(inst->op1_register);
			break;
		case OpType::MOFF:
			data.op1_size = get_size(inst->operand_size_override, inst->operand_byte_size_override);
			WARNING("moffs operands are not correctly addressed."); // TODO
			data.op1 = ram.read(inst->address_value, data.op1_size);
			break;
		case OpType::CREG:
			data.op1_size = OpSize::DW;
			data.op1 = registers.read_control_register(inst->op1_register);
			break;
		case OpType::NONE:
			break;
		}
	}
	
	if (inst->read_op2) {
		switch(inst->op2_type) {
		case OpType::REG:
			data.op2_size = get_size(inst->operand_size_override, inst->operand_byte_size_override);
			data.op2 = registers.read_index(inst->op2_register, data.op2_size);
			break;
		case OpType::MEM:
			data.op2_size = get_size(inst->address_size_override, inst->address_byte_size_override);
			data.op2 = ram.read(inst->address_value, data.op2_size);
			break;
		case OpType::IMM:
			data.op2_size = get_size(inst->operand_size_override, inst->operand_byte_size_override);
			data.op2 = inst->immediate_value;
			break;
		case OpType::M_M:
			// used only by BOUND
			// the fact that we are addressing memory more than once could be problematic
			data.op2_size = get_size(inst->address_size_override, 0);
			data.op2 = ram.read(inst->address_value, data.op2_size);
			if (data.op2_size == OpSize::DW) {
				data.op3 = ram.read(inst->address_value + 4, data.op2_size);
			}
			else {
				data.op3 = ram.read(inst->address_value + 2, data.op2_size);
			}
			data.op3_size = data.op2_size;
			break;
		case OpType::SREG:
			data.op2_size = OpSize::W;
			data.op2 = registers.read_segment(inst->op2_register);
			break;
		case OpType::MOFF:
			data.op2_size = get_size(inst->operand_size_override, inst->operand_byte_size_override);
			WARNING("moffs operands are not correctly addressed."); // TODO
			data.op2 = ram.read(inst->address_value, data.op1_size);
			break;
		case OpType::CREG:
			data.op2_size = OpSize::DW;
			data.op2 = registers.read_control_register(inst->op2_register);
			break;
		case OpType::NONE:
			break;
		}
	}

	// get the flags registers if needed
	U32 flags = 0;
	if (inst->get_flags) {
		flags = registers.EFLAGS;
	}
	
	// execute the instruction
	U32 return_value = 0;
	U32 return_value_2 = 0;
	if (inst->opcode & 0x80) { 
		// non-trivial op
		execute_non_arithmetic_instruction(inst);
	} else { 
		execute_arithmetic_instruction(inst->opcode, data, flags, return_value, return_value_2);
	}
	
	if (inst->get_flags) {
		// write the new flags
		registers.EFLAGS = flags;
	}

	// write the output of the instruction to its destination
	if (inst->write_ret1_to_register) {
		if (inst->scale_output_override) {
			registers.write_index(inst->register_out, return_value, data.op1_size);
		}
		else {
			// TODO: fix register addressing here, because we can only access 8 registers like this 
			registers.write(inst->register_out, return_value);
		}
	}
	else if (inst->write_ret1_to_op1) {
		switch (inst->op1_type) {
		case OpType::REG:
			registers.write_index(inst->op1_register, return_value, data.op1_size);
			break;
		case OpType::MEM:
			ram.write(inst->address_value, return_value, data.op1_size);
			break;
		default:
			break;
		}
	}
	
	if (inst->write_ret2_to_op2) {
		switch (inst->op2_type) {
		case OpType::REG:
			registers.write_index(inst->op2_register, return_value_2, data.op2_size);
			break;
		case OpType::MEM:
			ram.write(inst->address_value, return_value_2, data.op2_size);
			break;
		default:
			break;
		}
	}
}

void CPU::execute_arithmetic_instruction(const U8 opcode, const InstData data, U32& flags, U32& ret, U32& ret2)
{
	switch (opcode & 0x7F) // consider only the first 7 bits of the opcode
	{
	case Opcodes::AAA:
	{
		// if AF is set or if the first 4 bits of AL are greater than 9 
		if ((flags & Flags::AF) || ALU::compare_greater_or_equal(U8(data.op1) & 0x0F, 10)) {
			// we do
			// AL <- (AL + 6) and 0x0F
			// AH <- AH + 1
			// but in one step:
			// AX <- (AH + 1) : (AL + 6) & 0x0F
			// or:
			// AX <- (AX + 0x0106) & 0x0F0F <- 0F0F mask because we are doing BCD operations
			ret = ALU::add_no_carry(data.op1, (U16) 0x0106) & 0x0F0F;
			
			flags |= Flags::AF | Flags::CF;
		}
		else {
			ret = data.op1; // keep AX intact
			
			flags &= ~(Flags::AF | Flags::CF);
		}
		break;
	}
	case Opcodes::AAD:
	{
		U8 AH_val = (data.op1 & 0xFF00) >> 8;
		U8 AL_val = data.op1 & 0xFF;

		AL_val = ALU::add_no_carry(ALU::multiply(AH_val, (U8)10), AL_val);

		update_sign_flag(flags, AL_val, OpSize::B);
		update_zero_flag(flags, AL_val);
		update_parity_flag(flags, AL_val);
		
		ret = AL_val;
		break;
	}
	case Opcodes::AAM:
	{
		U8 q, r;
		bit divByZero;
		ALU::unsigned_divide(U8(data.op1 & 0xFF), (U8)10, q, r, divByZero);
		
		NYI; // TODO: those register calls should NOT be there!

		registers.write(Register::AH, q); 
		registers.write(Register::AL, r);

		update_sign_flag(flags, r, OpSize::B);
		update_zero_flag(flags, r);
		update_parity_flag(flags, r);
		
		ret = (q << 8) | r;
		break;
	}
	case Opcodes::AAS:
	{
		// if AF is set or if the first 4 bits of AL are greater than 9 
		if ((flags & Flags::AF) || ALU::compare_greater_or_equal(data.op1 & 0x0F, 10)) {
			// we do:
			// AL <- (AL - 6) & 0x0F
			// AH <- AH - 1
			// but in one step:
			// AX <- (AX + ((-1 << 8) + (-6 & 0x0F))) & 0xFF0F
			// notice: -6 & 0x0F = 0x0A
			// and then: (-1 << 8) + (-6 & 0x0F) = 0xFF0A
			ret = ALU::add_no_carry(U16(data.op1), (U16)0xFF0A) & 0xFF0F;
			
			flags |= Flags::AF | Flags::CF;
		}
		else {
			flags &= ~(Flags::AF | Flags::CF);
		}
		break;
	}
	case Opcodes::ADC:
	{
		bit carry = flags & Flags::CF;
		ret = ALU::add(data.op1, data.op2, carry);

		update_status_flags(flags, data.op1, data.op2, ret, data.op1_size, data.op2_size, data.op1_size, carry);
		break;
	}
	case Opcodes::ADD:
	{
		bit carry = 0;
		ret = ALU::add(data.op1, data.op2, carry);
		
		update_status_flags(flags, data.op1, data.op2, ret, data.op1_size, data.op2_size, data.op1_size, carry);
		break;
	}
	case Opcodes::AND:
	{
		ret = ALU::and_(data.op1, data.op2);
		
		flags &= ~(Flags::CF | Flags::OF);
		update_parity_flag(flags, ret);
		update_sign_flag(flags, ret, data.op1_size);
		update_zero_flag(flags, ret);
		break;
	}
	case Opcodes::ARPL:
	{
		// compare the first two bits of the operands, and branch if op1 < op2
		if (!ALU::compare_greater_or_equal(data.op1 & 0x03, data.op2 & 0x03)) {
			// copy the first two bits of op2 to the first two bits of op1
			ret = (data.op1 & (0xFF - 0x03)) | (data.op2 & 0x03);

			flags |= Flags::ZF;
		}
		else {
			flags &= Flags::ZF;
		}
		break;
	}
	case Opcodes::BOUND:
	{
		if (!ALU::compare_greater_or_equal(data.op1, data.op2) || ALU::compare_greater(data.op1, data.op3)) {
			INTERRUPT("#BR", 5);
		}
		break;
	}
	case Opcodes::BSF:
	{
		bit isZero;
		ret = ALU::get_first_set_bit_index(data.op1, isZero);

		if (isZero) {
			flags |= Flags::ZF;
		}
		else {
			flags &= ~Flags::ZF;
		}
		break;
	}
	case Opcodes::BSR:
	{
		bit isZero;
		ret = ALU::get_last_set_bit_index(data.op1, isZero);

		if (isZero) {
			flags |= Flags::ZF;
		}
		else {
			flags &= ~Flags::ZF;
		}
		break;
	}
	case Opcodes::BT:
	{
		if (data.op2 > 32) {
			// see the manual! (p. 268)
			WARNING("The BT instruction has an incomplete implementation for memomry operands.")
		}
		
		bit bit_val = ALU::get_bit_at(data.op1, data.op2 & 0b11111); // op2 mod 32
		if (bit_val) {
			flags |= Flags::CF;
		}
		else {
			flags &= ~Flags::CF;
		}
		break;
	}
	case Opcodes::BTC:
	{
		if (data.op2 > 32) {
			WARNING("The BTC instruction has an incomplete implementation for memomry operands.")
		}

		bit bit_val = ALU::get_bit_at(data.op1, data.op2, true);
		ret = data.op1;
		ALU::get_and_set_bit_at(ret, data.op2, !bit_val);
		if (bit_val) {
			flags |= Flags::CF;
		}
		else {
			flags &= ~Flags::CF;
		}
		break;
	}
	case Opcodes::BTR:
	{
		if (data.op2 > 32) {
			WARNING("The BTR instruction has an incomplete implementation for memomry operands.")
		}

		ret = data.op1;
		bit bit_val = ALU::get_and_set_bit_at(ret, data.op2, 0);
		if (bit_val) {
			flags |= Flags::CF;
		}
		else {
			flags &= ~Flags::CF;
		}
		break;
	}
	case Opcodes::BTS:
	{
		if (data.op2 > 32) {
			WARNING("The BTS instruction has an incomplete implementation for memomry operands.")
		}

		ret = data.op1;
		bit bit_val = ALU::get_and_set_bit_at(ret, data.op2, 1);
		if (bit_val) {
			flags |= Flags::CF;
		}
		else {
			flags &= ~Flags::CF;
		}
		break;
	}
	case Opcodes::CBW:
	{
		if (data.op1_size == OpSize::W) {
			U8 al = data.op1;
			ret = ALU::sign_extend((U16) al);
		}
		else {
			U16 ax = data.op1;
			ret = ALU::sign_extend((U32) ax);
		}
		break;
	}
	case Opcodes::CLC:
	{
		flags &= ~Flags::CF;
		break;
	}
	case Opcodes::CLD:
	{
		flags &= ~Flags::DF;
		break;
	}
	case Opcodes::CLI:
	{
		WARNING("Permission check missing, this instruction should sometimes raise an exception"); // TODO
		flags &= ~Flags::IF;
		break;
	}
	case Opcodes::CLTS:
	{
		// this instruction needs special treatment
		WARNING("Permission check missing, this instruction should sometimes raise an exception"); // TODO
		registers.control_flag_write_TS(0);
		break;
	}
	case Opcodes::CMC:
	{
		flags ^= Flags::CF;
		break;
	}
	case Opcodes::CMP:
	{
		bit carry;
		U32 val = ALU::sub(data.op1, data.op2, carry);
		
		update_status_flags(flags, data.op1, data.op2, val, data.op1_size, data.op2_size, data.op1_size, carry);
		break;
	}
	case Opcodes::CWD:
	{
		if (data.op2_size == OpSize::DW) {
			if (ALU::check_is_negative(data.op2, OpSize::DW)) {
				ret = 0xFFFFFFFF;
			}
			else {
				ret = 0;
			}
		}
		else {
			if (ALU::check_is_negative(data.op2, OpSize::W)) {
				ret = 0xFFFF;
			}
			else {
				ret = 0;
			}
		}
		break;
	}
	case Opcodes::DAA:
	{
		bit carry = 0;
		if ((flags & Flags::AF) || ALU::compare_greater_or_equal(U8(data.op1) & 0x0F, 10)) {
			ret = ALU::add(U8(data.op1), U8(6), carry); // AL += 6
			flags |= Flags::AF;
		}
		else {
			flags &= ~Flags::AF;
		}
		
		carry |= flags & Flags::CF;
		if (carry || ALU::compare_greater_or_equal(U8(data.op1), U8(0x10))) {
			ret = ALU::add_no_carry(U8(ret), U8(0x60)); // AL += 0x60
			flags |= Flags::CF;
		}
		else {
			flags &= ~Flags::CF;
		}
		
		update_parity_flag(flags, ret);
		update_sign_flag(flags, ret, data.op1_size);
		update_zero_flag(flags, ret);
		break;
	}
	case Opcodes::DAS:
	{
		bit carry = 0;
		if ((flags & Flags::AF) || ALU::compare_greater_or_equal(U8(data.op1 & 0x0F), U8(10))) {
			ret = ALU::sub(U8(data.op1), U8(6), carry); // AL -= 6
			flags |= Flags::AF;
		}
		else {
			flags &= ~Flags::AF;
		}
		
		carry |= flags & Flags::CF;
		if (carry || ALU::compare_greater_or_equal(U8(data.op1), U8(0x10))) {
			ret = ALU::sub_no_carry(U8(ret), U8(0x60)); // AL -= 0x60
			flags |= Flags::CF;
		}
		else {
			flags &= ~Flags::CF;
		}
		
		update_parity_flag(flags, ret);
		update_sign_flag(flags, ret, data.op1_size);
		update_zero_flag(flags, ret);
		break; 
	}
	case Opcodes::DEC:
	{
		ret = ALU::sub_no_carry(data.op1, U8(1));
		
		update_overflow_flag(flags, data.op1, U8(-1), ret, data.op1_size, OpSize::B, data.op1_size);
		update_parity_flag(flags, ret);
		update_sign_flag(flags, ret, data.op1_size);
		update_zero_flag(flags, ret);
		update_adjust_flag(flags, data.op1, -1); 
		break;
	}
	case Opcodes::DIV:
	{
		U32 r, q, n = data.op1, d = data.op2;
		bit divByZero = false;
		ALU::unsigned_divide(n, d, q, r, divByZero);
		
		if (divByZero) {
			INTERRUPT("#DE", 0);
		}
		
		ret = q;
		ret2 = r;
		break;
	}
	case Opcodes::IDIV:
	{
		U32 r, q, n = data.op1, d = data.op2;
		bit divByZero = false;
		ALU::signed_divide(n, d, q, r, divByZero);
		
		if (divByZero) {
			INTERRUPT("#DE", 0);
		}
		
		ret = q;
		ret2 = r;
		break;
	}
	case Opcodes::IMUL:
	{
		ret = ALU::multiply(data.op1, data.op2);

		update_overflow_flag(flags, data.op1, data.op2, ret, OpSize::UNKNOWN, OpSize::UNKNOWN, OpSize::UNKNOWN);
		break;
	}
	case Opcodes::IMULX:
	{
		// TODO: change this to perform full 64 bit multiplication instead
		// this implementation exactly what is not happening in the circuit implementation:
		// here we cast everything to 64 bits, perform 64 bit multiplication, 
		// and return the last 32 bits of the result.
		// In the circuit the circuit of this instruction is placed next to the circuit of IMUL,
		// and the carries from the adders are stored, and when using IMULX, 
		// we use them to compute the remaining 32 bits of the result.
		U64 a = I64(I32(data.op1)), b = I64(I32(data.op2)), r;
		r = ALU::multiply(a, b);
		ret = U32(r >> 32);

		// we can do this because we only care about the signs of the operands
		update_overflow_flag(flags, data.op1, data.op2, ret, OpSize::DW, OpSize::DW, OpSize::DW);
		break;
	}
	case Opcodes::INC:
	{
		ret = ALU::add_no_carry(data.op1, 1);
		
		update_overflow_flag(flags, data.op1, data.op2, ret, data.op1_size, data.op2_size, data.op1_size);
		update_sign_flag(flags, ret, data.op1_size);
		update_zero_flag(flags, ret);
		update_adjust_flag(flags, data.op1, data.op2);
		update_parity_flag(flags, ret);
		break;
	}
	case Opcodes::LAHF:
	{
		ret = flags & 0xFF;
		break;
	}
	case Opcodes::LEA:
	{
		ret = data.address;
		break;
	}
	// ------------ perfect implementation (I hope) up to here -------------
	case Opcodes::MOV:
	{
		ret = data.op2;
		// TODO : segment change checks, same for control registers, and other things...
		break;
	}
	case Opcodes::MOVSX:
	{
		ret = ALU::sign_extend(data.op2, data.op2_size);
		break;
	}
	case Opcodes::MOVZX:
	{
		ret = data.op2;
		break;
	}
	case Opcodes::MUL:
	{
		ret = ALU::multiply(data.op1, data.op2);

		// update the flags
		bit carry = 0;
		switch(data.op1_size)
		{
		case OpSize::B:
			carry = bool(ret & 0xFF00);
			break;
		case OpSize::W:
			carry = bool(ret & 0xFFFF0000);
			break;
		case OpSize::DW:
			throw BadInstruction("MUL does not support 64bit results. Use MULX.", registers.EIP);
		default:
			break; // handled in MULX
		}
		
		if (carry) {
			flags |= Flags::CF | Flags::OF;
		}
		else {
			flags &= ~(Flags::CF | Flags::OF);
		}
		break;
	}
	case Opcodes::MULX:
	{
		// in the circuit, the algorithm used to get a 64bit result is optimized
		// because the high bits of the operands are 0
		U64 a = data.op1, b = data.op2, r;
		r = ALU::multiply(a, b);
		ret = U32(r);
		ret2 = U32(r >> 32);
		
		bit carry = bool(ret2);
		if (carry) {
			flags |= Flags::CF | Flags::OF;
		}
		else {
			flags &= ~(Flags::CF | Flags::OF);
		}
		break;
	}
	case Opcodes::NEG:
	{
		ret = ALU::negate(data.op1);
		
		if (ALU::check_equal_zero(data.op1)) {
			flags |= Flags::CF;
		}
		else {
			flags &= ~Flags::CF;
		}
		
		flags &= ~Flags::OF; // no overflow is possible
		update_sign_flag(flags, ret, data.op1_size);
		update_zero_flag(flags, ret);
		update_parity_flag(flags, ret);
		break;
	}
	case Opcodes::NOP:
	{
		break;
	}
	case Opcodes::NOT:
	{
		ret = ALU::not_(data.op1);
		break;
	}
	case Opcodes::OR:
	{
		ret = ALU::or_(data.op1, data.op2);
		
		flags &= ~(Flags::CF | Flags::OF);
		update_sign_flag(flags, ret, data.op1_size);
		update_zero_flag(flags, ret);
		update_parity_flag(flags, ret);
		break;
	}
	case Opcodes::ROT:
	{
		// To encode this operation in one opcode, we use the immediate to specify the operation:
		// op1: dest, op2: source register value if any
		// op3: bit field with the count if any:
		//  - bits 0-4: count
		//  - 5: use the previous 5 bits as count instead of op2
		//  - 6: rotate left (else right)
		//  - 7: include the carry in the rotations (RC- instructions)
		U8 count = data.op3 & 0b11111;
		bit use_imm = data.op3 & (1 << 5);
		bit rot_left = data.op3 & (1 << 6);
		bit rot_carry = data.op3 & (1 << 7);
		
		if (!use_imm) {
			count = data.op2;
		}
		
		bit carry = flags & Flags::CF;
		if (rot_carry) {
			if (rot_left) {
				ret = ALU::rotate_left_carry(data.op1, carry, count, data.op1_size);
			}
			else {
				ret = ALU::rotate_right_carry(data.op1, carry, count, data.op1_size);
			}
		}
		else {
			if (rot_left) {
				ret = ALU::rotate_left(data.op1, carry, count, data.op1_size);
			}
			else {
				ret = ALU::rotate_right(data.op1, carry, count, data.op1_size);
			}
		}
		
		if (carry) {
			flags |= Flags::CF;
		}
		else {
			flags &= ~Flags::CF;
		}
		
		break;
	}
	/*
	case Opcodes::DAA:
	{
		break;
	}
	*/

	/*
	case Opcodes::XCHG:
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
	*/
	}
	
	registers.write_EIP(ALU::add_no_carry(registers.EIP, 1, true));
}

void CPU::execute_non_arithmetic_instruction(const Inst* inst)
{
	// TODO

	/*
	case Opcodes::CALL:
	{
		WARNING("The CALL instruction has an incomplete implementation, only near calls are implemented.")

			U32 eip = registers.read_EIP();
		push_4(eip);
		eip = ALU::add_no_carry(eip, op1_val);
		if (!is_32_bit_op_inst(inst->op1_size)) {
			eip &= 0xFFFF; // keep only the first 2 bytes
		}
		registers.write_EIP(eip);
		return; // skip EIP increment
	}
	case Opcodes::JO:
	{
		if (registers.flag_read_OF()) {
			registers.EIP = op1_val;
			return; // skip EIP increment
		}
		break;
	}
	case Opcodes::JNO:
	{
		if (!registers.flag_read_OF()) {
			registers.EIP = op1_val;
			return; // skip EIP increment
		}
		break;
	}
	*/
}

U32 CPU::compute_address(bit _32bits_mode, OpSize opSize) const
{
	U32 address = currentInstruction->address_value;
	U8 mod = currentInstruction->mod_rm_sib.mod;
	U8 rm = currentInstruction->mod_rm_sib.rm;
	
	if (mod == 0b11) {
		address = registers.read_index(rm, opSize);
	}
	else if (_32bits_mode) {
		if (rm == 0b100) {
			// use the SIB byte
			U8 scale = currentInstruction->mod_rm_sib.scale;
			U8 index = currentInstruction->mod_rm_sib.index;
			U8 base = currentInstruction->mod_rm_sib.base;
			
			U32 index_val = registers.read(index);
			
			// scale the index using chained shifters
			switch (scale)
			{
			case 0b11: index_val <<= 1; // *8
			case 0b10: index_val <<= 1; // *4
			case 0b01: index_val <<= 1; // *2
			default:   break;			// *1
			}
			
			address = ALU::add_no_carry(address, index_val);
			if (mod == 0b00 && base == 0b101) {
				// no base
			}
			else {
				U32 base_val = registers.read(base);
				address = ALU::add_no_carry(address, base_val);
			}
		}
		else if (mod == 0b00 && rm == 0b110) {
			// 'address' has already the right value
		}
		else {
			address = registers.read(rm); // only 32 bits registers are read here
		}
	}
	else { // 16 bits mode
		U16 a = 0, b = 0;
			
		switch (rm)
		{
			// TODO: this switch statememt cam greatly be simplified
				
		case 0b000:
			a = registers.read(Register::BX);
			b = registers.read(Register::SI);
			break;
				
		case 0b001:
			a = registers.read(Register::BX);
			b = registers.read(Register::DI);
			break;
				
		case 0b010:
			a = registers.read(Register::BP);
			b = registers.read(Register::SI);
			break;
				
		case 0b011:
			a = registers.read(Register::BP);
			b = registers.read(Register::DI);
			break;
				
		case 0b100:
			a = registers.read(Register::SI);
			break;
				
		case 0b101:
			a = registers.read(Register::DI);
			break;
			
		case 0b110:
			if (mod != 0b00) {
				a = registers.read(Register::BP);
			}
			break;
			
		case 0b111:
			a = registers.read(Register::BX);
			break;
		}
		
		address = ALU::add_no_carry(address, a);
		address = ALU::add_no_carry(address, b);
	}
	
	return address;
}

void CPU::push_2(U16 value)
{
	// TODO : maybe replace ESP incrementations in stack ops with one increment output from the instruction processor
	//  then we would have a dedicated adder next to the ESP register to perform one read, one add, one write.
	U32 esp = registers.read(Register::ESP);
	esp = ALU::add_no_carry(esp, (U32) -2);
	registers.write(Register::ESP, esp);
	
	stack.push(value);
}

void CPU::push_4(U32 value)
{
	U32 esp = registers.read(Register::ESP);
	esp = ALU::add_no_carry(esp, (U32) -4);
	registers.write(Register::ESP, esp);
	
	stack.push(value);
}

U16 CPU::pop_2()
{
	U16 val = stack.top();
	stack.pop();
	
	U32 esp = registers.read(Register::ESP);
	esp = ALU::add_no_carry(esp, (U32) 2);
	registers.write(Register::ESP, esp);
	
	return val;
}

U32 CPU::pop_4()
{
	U32 val = stack.top();
	stack.pop();
	
	U32 esp = registers.read(Register::ESP);
	esp = ALU::add_no_carry(esp, (U32) 4);
	registers.write(Register::ESP, esp);
	
	return val;
}

void CPU::update_overflow_flag(U32& flags, U32 op1, U32 op2, U32 result, OpSize op1Size, OpSize op2Size, OpSize retSize)
{
	/*
	overflow flag truth table (1: op1, 2: op2, R: result):
	1 2 R OF
	+ + + 0
	+ + - 1
	+ - / 0
	- + / 0
	- - - 0
	- - + 1

	=> (s(op1) & s(op2)) ^ s(R)
	*/
	bit is_op1_neg = ALU::check_is_negative(op1, op1Size, true);
	bit is_op2_neg = ALU::check_is_negative(op2, op2Size, true);
	bit is_ret_neg = ALU::check_is_negative(result, retSize, true);
	bit OF = (is_op1_neg & is_op2_neg) ^ is_ret_neg;
	if (OF) {
		flags |= Flags::OF;
	}
	else {
		flags &= ~Flags::OF;
	}
}

void CPU::update_sign_flag(U32& flags, U32 result, OpSize size)
{
	if (ALU::check_is_negative(result, size, true)) {
		flags |= Flags::SF;
	}
	else {
		flags &= ~Flags::SF;
	}
}

void CPU::update_zero_flag(U32& flags, U32 result)
{
	if (ALU::check_equal_zero(result, true)) {
		flags |= Flags::ZF;
	}
	else {
		flags &= ~Flags::ZF;
	}
}

void CPU::update_adjust_flag(U32& flags, U32 op1, U32 op2)
{
	/*
	Adjust flag is set only if there were an carry from the first 4 bits of the AL register to the 4 other bits.
	It is 0 otherwise, including when the operation didn't used the AL register.
	This function should only be called with instructions modifing the AL register (or AX and EAX, but not AH).
	*/
	if (currentInstruction->op1_type == OpType::REG && currentInstruction->op1_register == 0) {
		// Not the implementation used in the circuit, which is much simpler, 
		// as this flag comes out from the adder directly.
		bit AF = (op1 & 0x0F) + (op2 & 0x0F) > 0x0F;
		if (AF) {
			flags |= Flags::AF;
		}
		else {
			flags &= ~Flags::AF;
		}
	}
	else {
		flags &= ~Flags::AF;
	}
}

void CPU::update_parity_flag(U32& flags, U32 result)
{
	/*
	Parity check is made only on the first byte
	*/
	if (ALU::check_parity(U8(result & 0xFF))) {
		flags |= Flags::PF;
	}
	else {
		flags &= ~Flags::PF;
	}
}

void CPU::update_carry_flag(U32& flags, bit carry)
{
	if (carry) {
		flags |= Flags::PF;
	}
	else {
		flags &= ~Flags::PF;
	}
}

void CPU::update_status_flags(U32& flags, U32 op1, U32 op2, U32 result, OpSize op1Size, OpSize op2Size, OpSize retSize, bit carry)
{
	// updates all status flags
	update_overflow_flag(flags, op1, op2, result, op1Size, op2Size, retSize);
	update_sign_flag(flags, result, retSize);
	update_zero_flag(flags, result);
	update_adjust_flag(flags, op1, op2);
	update_parity_flag(flags, result);
	update_carry_flag(flags, carry);
}
