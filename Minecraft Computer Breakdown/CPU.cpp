
#include <iostream>
#include <any>

#include "ALU.hpp"
#include "CPU.h"
#include "exceptions.h"
#include "instructions.h"
#include "registers.h"

#define NYI throw NotImplemented(opcode, registers.EIP)

#define INTERRUPT(mnemonic, code) throw ProcessorExeception(mnemonic, registers.EIP, code)

CPU::CPU(const Inst** instructions, const U32 count)
	: instructions(instructions), instructions_count(count), currentInstruction(nullptr), interruptsTable(nullptr)
{
	// TODO: initialisation process
	//switch_protected_mode(); // use protected mode by default
}

void CPU::switch_protected_mode(bit protected_)
{
	// todo: see the startup process in the manual instead
	registers.control_flag_write_PE(protected_);
}

/**
 * @brief Executes the instructions until the end of the instructions list is reached.
*/
void CPU::run()
{
	const int max_cycles = 1000;
	while (registers.EIP < instructions_count) {
		new_clock_cycle();
		try {
			execute_instruction(); // increments the EIP register
		}
		catch (ExceptionWithMsg& e) {
			std::cerr << e.what() << "\n";
			break;
		}

		if (clock_cycle_count >= max_cycles) {
			std::cout << "Max cycles reached. Interrupting program.\n";
			break;
		}
	}

	std::cout << "Program finished in " << clock_cycle_count << " cycles.\n";
}

/**
 * @brief Utility to keep track of how many cycles elapsed
*/
void CPU::new_clock_cycle()
{
	clock_cycle_count++;
	// TODO : maybe reset the branch monitor here
}

/**
 * @brief Returns the size of an operand, using the prefixes of the instruction, as well as the D flag in the current segment.
*/
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

/**
 * @brief Executes the instruction at the address pointed by the EIP. 
 * Handles the common part of all instructions, which is operands fetching, flags update, and writing the results to their destination.
 * Actual instruction logic is delegated to CPU::execute_arithmetic_instruction(), CPU::execute_non_arithmetic_instruction() and
 * CPU::execute_non_arithmetic_instruction_with_state_machine().
*/
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

	// TODO : fetch the segment's D bit, maybe also store it and update it when there is a CS change
	OpSize operand_size = get_size(inst->operand_size_override, inst->operand_byte_size_override);
	OpSize address_size = get_size(inst->address_size_override, inst->address_byte_size_override);
	
	// TODO: use the computed address to read the operands. Also fix the operands size.
	
	// Read the operands
	U32 op1_val = 0;
	if (inst->read_op1) {
		switch(inst->op1_type) {
		case OpType::REG:
			data.op1_size = operand_size;
			data.op1 = registers.read_index(inst->op1_register, data.op1_size);
			break;
		case OpType::MEM:
			data.op1_size = address_size;
			data.op1 = ram.read(inst->address_value, data.op1_size);
			break;
		case OpType::IMM:
			data.op1_size = operand_size;
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
			data.op1_size = operand_size;
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
			data.op2_size = operand_size;
			data.op2 = registers.read_index(inst->op2_register, data.op2_size);
			break;
		case OpType::MEM:
			data.op2_size = address_size;
			data.op2 = ram.read(inst->address_value, data.op2_size);
			break;
		case OpType::IMM:
			data.op2_size = operand_size;
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
			data.op2_size = operand_size;
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
	if (inst->opcode & Opcodes::not_arithmethic) { 
		// non-trivial op

		data.op_size = operand_size;
		data.ad_size = address_size;

		if (inst->opcode & Opcodes::state_machine) {
			// include all state machine, jump and string instructions
			execute_non_arithmetic_instruction_with_state_machine(inst->opcode, data, flags);
		}
		else {
			execute_non_arithmetic_instruction(inst->opcode, data, flags, return_value, return_value_2);		
		}
	}
	else {
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

/**
 * @brief Executes the instruction specified by its opcode. This method handles all 'simple' instructions.
 * @param data Holds instruction information
 * @param flags EFLAGS register
 * @param ret Return value of the instruction
 * @param ret2 Additionnal return value of the instruction
*/
void CPU::execute_arithmetic_instruction(const U8 opcode, const InstData data, U32& flags, U32& ret, U32& ret2)
{
	switch (opcode) // we could consider only the first 7 bits of the opcode
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

		AL_val = ALU::add_no_carry(ALU::multiply_no_overflow(AH_val, (U8)10), AL_val);

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
		
		ret = (q << 8) | r;

		update_sign_flag(flags, r, OpSize::B);
		update_zero_flag(flags, r);
		update_parity_flag(flags, r);
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
			WARNING("The BT instruction has an incomplete implementation for memory operands.")

			/*
			16 bits:
				15-4 3-0
				|	 |-> index of the bit to use from the 2 bytes fetched from memory
				|-> additionnal offset for the memory operand, x2

			32 bits:
				31-5 4-0
				|	 |-> index of the bit to use from the 4 bytes fetched from memory
				|-> additionnal offset for the memory operand, x4
			*/
		}
		
		bit bit_val = ALU::get_bit_at(data.op1, data.op2 & 0b11111); // op2 mod 32
		update_carry_flag(flags, bit_val);
		break;
	}
	case Opcodes::BTC:
	{
		if (data.op2 > 32) {
			WARNING("The BTC instruction has an incomplete implementation for memory operands.")
		}

		bit bit_val = ALU::get_bit_at(data.op1, data.op2, true);
		ret = data.op1;
		ALU::get_and_set_bit_at(ret, data.op2, !bit_val);
		update_carry_flag(flags, bit_val);
		break;
	}
	case Opcodes::BTR:
	{
		if (data.op2 > 32) {
			WARNING("The BTR instruction has an incomplete implementation for memory operands.")
		}

		ret = data.op1;
		bit bit_val = ALU::get_and_set_bit_at(ret, data.op2, 0);
		update_carry_flag(flags, bit_val);
		break;
	}
	case Opcodes::BTS:
	{
		if (data.op2 > 32) {
			WARNING("The BTS instruction has an incomplete implementation for memory operands.")
		}

		ret = data.op1;
		bit bit_val = ALU::get_and_set_bit_at(ret, data.op2, 1);
		update_carry_flag(flags, bit_val);
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
		bit overflow = 0;
		ret = ALU::multiply(data.op1, data.op2, overflow);

		// set both the carry and the overflow flag
		if (overflow) {
			flags |= Flags::OF | Flags::CF;
		}
		else {
			flags &= ~(Flags::OF | Flags::CF);
		}
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
		bit overflow = 0;
		r = ALU::multiply(a, b, overflow);
		ret = U32(r >> 32);

		// set both the carry and the overflow flag
		if (overflow) {
			flags |= Flags::OF | Flags::CF;
		}
		else {
			flags &= ~(Flags::OF | Flags::CF);
		}
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
		bit overflow = 0;
		ret = ALU::multiply(data.op1, data.op2, overflow);

		// TODO : remove this if the overflow flag works
		/*
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
		*/
		if (data.op1_size == OpSize::DW) { // TODO : change? remove?
			throw BadInstruction("MUL does not support 64bit results. Use MULX.", registers.EIP);
		}
		
		// set both the carry and the overflow flag
		if (overflow) {
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
		bit overflow = 0;
		r = ALU::multiply(a, b, overflow);
		ret = U32(r);
		ret2 = U32(r >> 32);
		
		// set both the carry and the overflow flag
		if (overflow) {
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

        U8 last_bit_pos = 0;
		switch (data.op1_size)
		{
		case OpSize::DW: last_bit_pos = sizeof(U32) * 8 - 1; break;
		case OpSize::W:  last_bit_pos = sizeof(U16) * 8 - 1; break;
		case OpSize::B:  last_bit_pos = sizeof(U8)  * 8 - 1; break;
        case OpSize::UNKNOWN: throw BadInstruction("Incorrect Operand Size", registers.EIP);
		}
		
		bit carry = flags & Flags::CF;
        bit overflow = 0;
		if (rot_carry) {
			if (rot_left) {
				ret = ALU::rotate_left_carry(data.op1, carry, count, data.op1_size);
				overflow = carry != bit(ret & (1 << last_bit_pos));
			}
			else {
				ret = ALU::rotate_right_carry(data.op1, carry, count, data.op1_size);
				overflow = bit(ret & (1 << last_bit_pos)) != bit(ret & (1 << (last_bit_pos - 1)));
			}
		}
		else {
			if (rot_left) {
				ret = ALU::rotate_left(data.op1, carry, count, data.op1_size);
				overflow = carry != bit(ret & (1 << last_bit_pos));
			}
			else {
				ret = ALU::rotate_right(data.op1, carry, count, data.op1_size);
				overflow = bit(ret & (1 << last_bit_pos)) != bit(ret & (1 << (last_bit_pos - 1)));
			}
		}
		
		update_overflow_flag_from_bit(flags, overflow);
		update_carry_flag(flags, carry);
		break;
	}
    case Opcodes::SAHF:
	{
        // store only SF, ZF, AF, PF and CF from AH
        flags = (flags & 0xFF) | (data.op1 & 0b11010101);
		break;
	}
    case Opcodes::SHFT:
    {
        // To encode this operation in one opcode, we use the immediate to specify the operation:
		// op1: dest, op2: source register value if any
		// op3: bit field with the count if any:
		//  - bits 0-4: count
		//  - 5: use the previous 5 bits as count instead of op2
		//  - 6: shift left (else right)
        //  - 7: keep the sign (only for right shifts)
		U8 count = data.op3 & 0b11111;
		bit use_imm = data.op3 & (1 << 5);
		bit shift_left = data.op3 & (1 << 6);
        bit keep_sign = data.op3 & (1 << 7);

        if (!use_imm) {
            count = data.op2;
        }

        U8 last_bit_pos = 0;
		switch (data.op1_size)
		{
		case OpSize::DW: last_bit_pos = sizeof(U32) * 8 - 1; break;
		case OpSize::W:  last_bit_pos = sizeof(U16) * 8 - 1; break;
		case OpSize::B:  last_bit_pos = sizeof(U8)  * 8 - 1; break;
        case OpSize::UNKNOWN: throw BadInstruction("Incorrect Operand Size", registers.EIP);
		}

        bit carry = 0;
        bit overflow = 0;
        if (shift_left) {
            ret = ALU::shift_left(data.op1, carry, count, data.op1_size);
			overflow = bool(ret & (1 << last_bit_pos)) != carry;
        }
        else {
            ret = ALU::shift_right(data.op1, carry, count, data.op1_size, keep_sign);
            if (keep_sign) {
				overflow = 0;
            }
            else {
				overflow = bool(data.op1 & (1 << last_bit_pos));
            }
        }

		update_overflow_flag_from_bit(flags, overflow);
		update_carry_flag(flags, carry);
        update_sign_flag(flags, ret, data.op1_size);
		update_zero_flag(flags, ret);
		update_parity_flag(flags, ret);
		break;
    }
    case Opcodes::SBB:
	{
		U32 op2 = ALU::sign_extend(data.op2, data.op2_size);
		
		bit carry = bool(flags & Flags::CF);
		op2 = ALU::add(op2, 0, carry);
		ret = ALU::sub(data.op1, op2, carry);
		
		update_status_flags(flags, data.op1, data.op2, ret, data.op1_size, data.op2_size, data.op1_size, carry);
		break;
	}
	case Opcodes::SETcc:
	{
		U8 condition = data.imm;
		switch (condition)
		{
		case 0b0000: // Above | Not below or equal
			// ZF = 0 && CF = 0
			ret = !bool(flags & (Flags::ZF | Flags::CF));
			break;
			
		case 0b0001: // Above or equal | Not below | Not carry
			// CF = 0
			ret = !bool(flags & Flags::CF);
			break;
			
		case 0b0010: // Below | Carry | Not above or equal
			// CF = 1
			ret = bool(flags & Flags::CF);
			break;
			
		case 0b0011: // Below or equal | Not above
			// ZF = 1 || CF = 1
			ret = bool(flags & Flags::ZF) | bool(flags & Flags::CF);
			break;
			
		case 0b0100: // Equal | Zero
			// ZF = 1
			ret = bool(flags & Flags::ZF);
			break;
			
		case 0b0101: // Greater | Not less or equal
			// ZF = 0 || (SF == OF)
			ret = !bool(flags & Flags::ZF) || !(bool(flags & Flags::SF) ^ bool(flags & Flags::OF));
			break;
			
		case 0b0110: // Greater or Equal | Not less
			// SF == OF
			ret = !(bool(flags & Flags::SF) ^ bool(flags & Flags::OF));
			break;
			
		case 0b0111: // Less | Not greater or equal
			// SF != OF
			ret = bool(flags & Flags::SF) ^ bool(flags & Flags::OF);
			break;
			
		case 0b1000: // Less or equal | Not greater
			// ZF = 1 && SF != OF
			ret = bool(flags & Flags::ZF) & (bool(flags & Flags::SF) ^ bool(flags & Flags::OF));
			break;
			
		case 0b1001: // Not equal | Not zero
			// ZF = 0
			ret = !bool(flags & Flags::ZF);
			break;
			
		case 0b1010: // Not overflow
			// OF = 0
			ret = !bool(flags & Flags::OF);
			break;
		
		case 0b1011: // Not parity | Parity odd
			// PF = 0
			ret = !bool(flags & Flags::PF);
			break;
			
		case 0b1100: // Not sign
			// SF = 0
			ret = !bool(flags & Flags::SF);
			break;
			
		case 0b1101: // Overflow
			// OF = 1
			ret = bool(flags & Flags::OF);
			break;
		
		case 0b1110: // Parity | Parity even
			// PF = 1
			ret = bool(flags & Flags::PF);
			break;
			
		case 0b1111: // Sign
			// SF = 1
			ret = bool(flags & Flags::SF);
			break;
		}
		break;
	}
	case Opcodes::SHD:
	{
		U8 count = data.op3 & 0b11111;
		bit shift_left = data.op3 & (1 << 5);
		
		bit carry = 0;
		if (shift_left) {
			U64 merged = U64(data.op1) << 32;
			if (data.op2_size == OpSize::DW) {
				merged |= data.op2;
			}
			else { // W operand
				merged |= static_cast<U64>(U16(data.op2)) << 16;
			}
			merged = ALU::shift_left(merged, carry, count);
			ret = U32(merged >> 32);
		}
		else {
			U64 merged = U64(data.op2) << 32;
			if (data.op1_size == OpSize::DW) {
				merged |= data.op1;
			}
			else { // W operand
				merged |= static_cast<U64>(U16(data.op1)) << 16;
			}
			merged = ALU::shift_right(merged, carry, count);
			if (data.op1_size == OpSize::DW) {
				ret = U32(merged);
			}
			else {
				ret = U32(merged >> 16);
			}
		}
		
		update_carry_flag(flags, carry);
        update_sign_flag(flags, ret, data.op1_size);
		update_zero_flag(flags, ret);
		update_parity_flag(flags, ret);
		break;
	}
	case Opcodes::STC:
	{
		flags |= Flags::CF;
		break;
	}
	case Opcodes::STD:
	{
		flags |= Flags::DF;
		break;
	}
	case Opcodes::STI:
	{
		// TODO: permission checks
		flags |= Flags::IF;
		break;
	}
	case Opcodes::SUB:
	{
		U32 op2 = ALU::sign_extend(data.op2, data.op2_size);
		
		bit carry = 0;
		ret = ALU::sub(data.op1, op2, carry);
		
		update_status_flags(flags, data.op1, data.op2, ret, data.op1_size, data.op2_size, data.op1_size, carry);
		break;
	}
	case Opcodes::TEST:
	{
		U32 result = data.op1 & data.op2;
		
		flags &= ~(Flags::OF | Flags::CF); // clear the OF and CF flags
		update_parity_flag(flags, result);
		update_sign_flag(flags, result, data.op1_size);
		update_zero_flag(flags, result);
		break;
	}
	case Opcodes::XCHG:
	{
		ret = data.op2;
		ret2 = data.op1;
		break;
	}
	case Opcodes::XLAT:
	{
		// TODO : make sure to make this instruction use 32bit addressing. 
		//  In order to not have to fetch something from memory here, but before this instruction is executed,
		//  The Mod r/m and SIB bytes should: add EBX and AL together to have the offset in the segment 
		//  stored in DS to get one single byte in memory.
		ret = data.op1;
		break;
	}
	case Opcodes::XOR:
	{
		ret = ALU::xor_(data.op1, data.op2);
		// TODO: flags
		break;
	}
	default:
	{
		throw UnknownInstruction("Unknown arithmetic instruction", opcode, registers.EIP);
	}
	}

	registers.write_EIP(ALU::add_no_carry(registers.EIP, 1, true));
}

/**
 * @brief  Executes the instruction specified by its opcode. This method handles special instructions which needs to access or modify special data,
 * like stack operations, I/O, descriptor table operations...
 * @param data Holds instruction information
 * @param flags EFLAGS register
 * @param ret Return value of the instruction
 * @param ret2 Additionnal return value of the instruction
*/
void CPU::execute_non_arithmetic_instruction(const U8 opcode, const InstData data, U32& flags, U32& ret, U32& ret2)
{
	// TODO : check if ret and ret2 are both needed
	
	switch (opcode) // we could consider only the first 7 bits of the opcode
	{
	case Opcodes::HLT:
	{
		// TODO ? (here is also a priviliege check to do)
		break;
	}
	case Opcodes::IN:
	{
		U8 CPL = 0; // TODO : get CPL from the segment descriptor
		if (registers.control_flag_read_PE() && (ALU::compare_greater(CPL, U8((flags & Flags::IOPL) >> 12)) || (flags & Flags::VM))) {
			// We are in protected mode, with CPL > IOPL or in virtual mode
			// TODO : default IO permission bit value: 0
			if (0) {
				INTERRUPT("#GP", 0);
			}
		}
		ret = io.read(data.op2, data.op1_size);
		break;
	}
	case Opcodes::LAR:
	{
		// TODO : this is an instruction using the descriptor table
		break;
	}
	case Opcodes::LGDT:
	{
		// TODO : this is an instruction using the descriptor table
		break;
	}
	case Opcodes::LGS:
	{
		// TODO : this is an instruction using the descriptor table (I think)
		break;
	}
	case Opcodes::LLDT:
	{
		// TODO : this is an instruction using the descriptor table
		break;
	}
	case Opcodes::LMSW:
	{
		// TODO : permission check
		// load only the first 6 bits
		registers.control_registers[0] = (registers.control_registers[0] & 0b00111111) | data.op1;
		break;
	}
	case Opcodes::LOCK:
	{
		// TODO : I don't know what to do here
		break;
	}
	case Opcodes::LSL:
	{
		// TODO : this is an instruction using the descriptor table
		break;
	}
	case Opcodes::LTR:
	{
		// TODO : I don't know where the task register is
		break;
	}
	case Opcodes::OUT:
	{
		U8 CPL = 0; // TODO : get CPL from the segment descriptor
		if (registers.control_flag_read_PE() && (ALU::compare_greater(CPL, U8((flags & Flags::IOPL) >> 12)) || (flags & Flags::VM))) {
			// We are in protected mode, with CPL > IOPL or in virtual mode
			// TODO : default IO permission bit value: 0
			if (0) {
				throw ProcessorExeception("#GP", registers.EIP, 0);
			}
		}
		write_io(data.op1, data.op2, data.op2_size);
		break;
	}
	case Opcodes::POP:
	{
		ret = pop(data.op1_size);
		// TODO : writing to segment registers is possible here, checks should be made
		break;
	}
	case Opcodes::POPF:
	{
		if (data.op_size == OpSize::DW) {
			flags = pop(OpSize::DW);
		}
		else {
			flags = (flags & 0xFF00) | pop(OpSize::W);
		}
		break;
	}
	case Opcodes::PUSH:
	{
		push(data.op1, data.op1_size);
		// TODO : reading from segment registers is possible here, checks should be made
		break;
	}
	case Opcodes::PUSHF:
	{
		if (data.op_size == OpSize::DW) {
			push(flags, OpSize::DW);
		}
		else {
			push(flags & 0xFFFF, OpSize::W);
		}
		break;
	}
	case Opcodes::SGDT:
	{
		// TODO : this is an instruction using the descriptor table
		break;
	}
	case Opcodes::SLDT:
	{
		// TODO : this is an instruction using the descriptor table
		break;
	}
	case Opcodes::SMSW:
	{
		// TODO : permission check
		// get only the first 6 bits
		ret = registers.control_registers[0] & 0b00111111;
		break;
	}
	case Opcodes::STR:
	{
		// TODO : I don't know where the task register is
		break;
	}
	case Opcodes::VERR:
	{
		// TODO : this is an instruction using the descriptor table
		break;
	}
	case Opcodes::WAIT:
	{
		// TODO ? (here is also a priviliege check to do, I think)
		break;
	}
	default:
	{
		throw UnknownInstruction("Unknown Non-arithmetic instruction", opcode, registers.EIP);
	}
	}
	
	registers.write_EIP(ALU::add_no_carry(registers.EIP, 1, true));
}

/**
 * @brief Handles more complex instructions, which may require serveral clock cycles to execute, or modifies the instruction pointer explicitly.
 * @param data Holds instruction information
 * @param flags EFLAGS register
*/
void CPU::execute_non_arithmetic_instruction_with_state_machine(const U8 opcode, const InstData data, U32& flags)
{
	// All parameters are stored on pseudo registers which are read
	// each loop. Their values cannot change during execution.
	// When the instruction is completed, 'stop' is set and all
	// data stored in the instruction should be reset.
	// Each instruction should be fought as a state machine, with
	// the index as the main parameter.
	// This design allows for instructions which last serveral cycles,
	// without the need to decode the instruction and transmit its
	// parameters at each cycle. Memory, registers, stack, and special
	// data storage can be read/written to several times in one
	// instruction (but only once for each cycle).
	
	bit incr_EIP = !(opcode & Opcodes::jmp) || (opcode & Opcodes::str); // do not increment EIP automatically for jump only instructions
	bit repeat = false;
	U8 state = 0, incr_state = 0;
	U8 index = 0, incr_index = 0;

	// in the circuit implementation, each instruction circuit has its own custom storage
	union Storage
	{
		struct NoStorage
		{} no_storage;

		struct Enter
		{
			U32 frame_ptr;
			U32 ebp;
		} enter;
		
		struct PUSHA
		{
			U32 esp;
		} pusha;
		
	} storage{};
	
	// Yes there is no indentation. What are you going to do about this huh?
	do
	{
	incr_state = 0; // TODO : remove state var if not used
	incr_index = 0;
	repeat = 0;
	switch (opcode) // we could consider only the first 7 bits of the opcode
	{
	case Opcodes::CALL:
	{
		WARNING("The CALL instruction has an incomplete implementation, only near calls are implemented.")

		U32 eip = registers.read_EIP();
		push(eip, OpSize::DW); // TODO : I think that this is a DW
		eip = ALU::add_no_carry(eip, data.op1);
		if (data.op1_size == OpSize::W) {
			eip &= 0xFFFF; // keep only the first 2 bytes
		}
		registers.write_EIP(eip);
		break;
	}
	case Opcodes::ENTER:
	{
		switch(state)
		{
		case 0:
			// init
			storage.enter = {
				registers.read(Register::EBP), // ebp
				registers.read(Register::ESP)  // frame_ptr
			};
			push(storage.enter.ebp);
			if (ALU::check_equal_zero(data.op2)) {
				state = 2;
			}
			else {
				state = 1;
			}
			repeat = 1;
			break;
		
		case 1:
		{
			if (ALU::compare_greater(index, U8(data.op2))) {
				// stack frame is finished
				push(storage.enter.frame_ptr);
				state = 2;
			}
			else {
				// build stack frame levels
				if (data.op_size == OpSize::DW) {
					storage.enter.ebp = ALU::sub_no_carry(storage.enter.ebp, 4);
					push(ram.read(storage.enter.ebp, data.op_size), data.op_size);
				}
				else {
					storage.enter.ebp = ALU::sub_no_carry(storage.enter.ebp, 2);
					push(ram.read(storage.enter.ebp, data.op_size), data.op_size);
				}
				incr_index = 1;
			}
			repeat = 1;
			break;
		}
		
		case 2:
			// push frame-ptr, and make space for locals variables
			registers.write(Register::EBP, storage.enter.frame_ptr);
			registers.write(Register::ESP, data.op1);
			break;
		}
		break;
	}
	case Opcodes::Jcc:
	{
		U8 condition = data.imm;
		bit jump = 0;
		switch (condition)
		{
		case 0b00000: // Above | Not below or equal
			// ZF = 0 && CF = 0
			jump = !bool(flags & (Flags::ZF | Flags::CF));
			break;

		case 0b00001: // Above or equal | Not below | Not carry
			// CF = 0
			jump = !bool(flags & Flags::CF);
			break;

		case 0b00010: // Below | Carry | Not above or equal
			// CF = 1
			jump = bool(flags & Flags::CF);
			break;

		case 0b00011: // Below or equal | Not above
			// ZF = 1 || CF = 1
			jump = bool(flags & Flags::ZF) | bool(flags & Flags::CF);
			break;

		case 0b00100: // Equal | Zero
			// ZF = 1
			jump = bool(flags & Flags::ZF);
			break;

		case 0b00101: // Greater | Not less or equal
			// ZF = 0 || (SF == OF)
			jump = !bool(flags & Flags::ZF) || ~(bool(flags & Flags::SF) ^ bool(flags & Flags::OF));
			break;

		case 0b00110: // Greater or Equal | Not less
			// SF == OF
			jump = !(bool(flags & Flags::SF) ^ bool(flags & Flags::OF));
			break;

		case 0b00111: // Less | Not greater or equal
			// SF != OF
			jump = bool(flags & Flags::SF) ^ bool(flags & Flags::OF);
			break;

		case 0b01000: // Less or equal | Not greater
			// ZF = 1 && SF != OF
			jump = bool(flags & Flags::ZF) & (bool(flags & Flags::SF) ^ bool(flags & Flags::OF));
			break;

		case 0b01001: // Not equal | Not zero
			// ZF = 0
			jump = !bool(flags & Flags::ZF);
			break;

		case 0b01010: // Not overflow
			// OF = 0
			jump = !bool(flags & Flags::OF);
			break;

		case 0b01011: // Not parity | Parity odd
			// PF = 0
			jump = !bool(flags & Flags::PF);
			break;

		case 0b01100: // Not sign
			// SF = 0
			jump = !bool(flags & Flags::SF);
			break;

		case 0b01101: // Overflow
			// OF = 1
			jump = bool(flags & Flags::OF);
			break;

		case 0b01110: // Parity | Parity even
			// PF = 1
			jump = bool(flags & Flags::PF);
			break;

		case 0b01111: // Sign
			// SF = 1
			jump = bool(flags & Flags::SF);
			break;

		case 0b10000: // CX register is zero
			// CX = 0
			jump = bool(ALU::check_equal_zero(U16(registers.read(Register::CX))));
			break;

		case 0b10001: // ECX register is zero
			// ECX = 0
			jump = bool(ALU::check_equal_zero(registers.read(Register::ECX)));
			break;
		}

		if (jump) {
			U32 new_eip = ALU::add_no_carry(registers.read_EIP(), data.op1);
			if (data.op_size == OpSize::W) {
				new_eip = U16(new_eip);
			}
			registers.write_EIP(new_eip);
		}

		break;
	}
	case Opcodes::POPA:
	{
		U32 val = pop(data.op_size);
		if (ALU::compare_equal(index, U8(4))) {
			// skip SP/ESP
		}
		else {
			U8 reversed_index = ALU::sub_no_carry(U8(7), index);
			registers.write(reversed_index, val);
		}
		
		if (!ALU::compare_equal(index, U8(7))) {
			// continue for each register
			incr_index = 1;
			repeat = 1;
		}
		break;
	}
	case Opcodes::PUSHA:
	{
		if (ALU::check_equal_zero(index)) {
			storage.pusha.esp = registers.read_index(Register::ESP, data.op_size);
		}
		
		if (ALU::compare_equal(index, U8(3))) {
			// handle SP/ESP
			push(storage.pusha.esp, data.op_size);
		}
		else {
			U32 val = registers.read_index(index, data.op_size);
			push(val, data.op_size);
		}
		
		if (!ALU::compare_equal(index, U8(7))) {
			// continue for each register
			incr_index = 1;
			repeat = 1;
		}
		break;
	}
	default:
	{
		throw UnknownInstruction("Unknown Non-arithmetic state machine instruction", opcode, registers.EIP);
	}
	}
	// dedicated incrementation zone
	if (incr_state) {
		state = ALU::add_no_carry(state, U8(1));
	}
	if (incr_index) {
		index = ALU::add_no_carry(index, U8(1));
	}
	if (repeat) {
		new_clock_cycle();
	}
	} while (repeat);
	
	if (incr_EIP) {
		registers.write_EIP(ALU::add_no_carry(registers.EIP, 1, true));
	}
}

/**
 * @brief Computes the effective address of the address operand of the current instruction.
*/
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
			case 0b11: index_val <<= 1; [[fallthrough]]; // *8
			case 0b10: index_val <<= 1; [[fallthrough]]; // *4
			case 0b01: index_val <<= 1; [[fallthrough]]; // *2
			default:   break;							 // *1
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

/**
 * @brief Push a value to the stack, of variable size.
 * @param value The value to push
 * @param size The size of the value
*/
void CPU::push(U32 value, OpSize size)
{
	U32 esp = registers.read(Register::ESP);
	
	if (size == OpSize::UNKNOWN) {
		size = get_size(currentInstruction->operand_size_override, 0);
	}
		
	switch (size)
	{
	case OpSize::DW:
		esp = ALU::add_no_carry(esp, (U32) -4);
		break;
		
	case OpSize::W:
		esp = ALU::add_no_carry(esp, (U32) -2);
		break;
		
	default:
		throw BadInstruction("Wrong operand size for push", registers.EIP);
	}
	
	registers.write(Register::ESP, esp);
	
	stack.push(value);
}

/**
 * @brief Pop a value from the stack.
 * @param size The size of the value.
 * @return The value popped
*/
U32 CPU::pop(OpSize size)
{
	U32 val = stack.top();
	stack.pop();
	
	U32 esp = registers.read(Register::ESP);
	
	if (size == OpSize::UNKNOWN) {
		size = get_size(currentInstruction->operand_size_override, 0);
	}
		
	switch (size)
	{
	case OpSize::DW:
		esp = ALU::add_no_carry(esp, (U32) 4);
		break;
		
	case OpSize::W:
		esp = ALU::add_no_carry(esp, (U32) 2);
		break;
		
	default:
		throw BadInstruction("Wrong operand size for pop", registers.EIP);
	}
	
	registers.write(Register::ESP, esp);
	
	return val;
}

/**
 * @brief Called to trigger an interrupt: saves the position in the program, setup the stack and error code data,
 * then switch to the corresponding interrupt handler. There is serveral things we need push to the stack, so we must
 * use the state machine design in order to spread them in serveral clock cycles.
 * @param interrupt Interrupt info
*/
void CPU::interrupt(Interrupts::Interrupt interrupt)
{
	U8 index = 0;
	bit repeat = 0, incr_index = 0;

	do {
		repeat = 0;
		incr_index = 0;

		switch (interrupt.type)
		{
		case Interrupts::Type::Fault:
			// at
			// TODO : interrupts, see the 1986 manual at page 160, or the most recent one at page 3006
			break;

		case Interrupts::Type::Trap:
			// after
			break;

		case Interrupts::Type::Abort:
			break;

		case Interrupts::Type::User:
			// after
			break;

		default:
			break;
		}

		if (incr_index) {
			index = ALU::add_no_carry(index, U8(1));
		}
		if (repeat) {
			new_clock_cycle();
		}
	} while (repeat);
}

/**
 * @brief Utility function used to compute and update the value of the overflow flag, after a addition or substraction.
 * To do this we need both the operands and the result, as well as their sizes.
*/
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

	=> (sign(op1) & sign(op2)) ^ sign(R)
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

/**
 * @brief Utility function used to update the value of the overflow flag, from an already computed flag.
*/
void CPU::update_overflow_flag_from_bit(U32& flags, bit overflow)
{
	if (overflow) {
		flags |= Flags::OF;
	}
	else {
		flags &= ~Flags::OF;
	}
}

/**
 * @brief Utility function used to update the value of the sign flag based on the sign of the result.
*/
void CPU::update_sign_flag(U32& flags, U32 result, OpSize size)
{
	if (ALU::check_is_negative(result, size, true)) {
		flags |= Flags::SF;
	}
	else {
		flags &= ~Flags::SF;
	}
}

/**
 * @brief Utility function used to update the value of the zero flag.
*/
void CPU::update_zero_flag(U32& flags, U32 result)
{
	if (ALU::check_equal_zero(result, true)) {
		flags |= Flags::ZF;
	}
	else {
		flags &= ~Flags::ZF;
	}
}

/**
 * @brief Utility function used to update the value of the adjust flag, after a arithmetic operation using the value of the AL register.
*/
void CPU::update_adjust_flag(U32& flags, U32 op1, U32 op2)
{
	/*
	Adjust flag is set only if there were an carry from the first 4 bits of the AL register to the 4 other bits.
	It is 0 otherwise, including when the operation didn't used the AL register.
	This function should only be called with instructions modifing the AL register (or AX and EAX, but not AH).
	*/
	if (currentInstruction->op1_type == OpType::REG && currentInstruction->op1_register == 0) {
		// Not the implementation used in the circuit, which is much simpler, 
		// as this flag can come out from the adder directly.
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

/**
 * @brief Utility function used to update the value of the parity flag.
*/
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

/**
 * @brief Utility function used to update the value of the carry flag.
*/
void CPU::update_carry_flag(U32& flags, bit carry)
{
	if (carry) {
		flags |= Flags::CF;
	}
	else {
		flags &= ~Flags::CF;
	}
}

/**
 * @brief Utility function used to update all arithmetic flags
*/
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

/**
 * @brief Read bytes from the IO buffer
*/
U32 CPU::read_io(U8 io_address, OpSize size)
{
    return io.read(io_address, size);
}

/**
 * @brief Write bytes to the IO buffer
*/
void CPU::write_io(U8 io_address, U32 value, OpSize size)
{
    io.write(io_address, value, size);
}
