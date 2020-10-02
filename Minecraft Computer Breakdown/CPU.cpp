
#include <iostream>

#include "CPU.h"

#define NYI throw NotImplemented(opcode, registers.EIP)

#define INTERRUPT(mnemonic, code) throw ProcessorExeception(mnemonic, registers.EIP, code)

CPU::CPU(const Inst** instructions, const U32 count)
	: instructions(instructions), instructions_count(count) 
{
	switch_protected_mode(); // use protected mode by default
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

U32 CPU::inst_get_operand(U8 register_index, bit operand_size_override, bit operand_byte_size_override) const
{
	U32 val;
	if (operand_byte_size_override) {
		// 8 bit value
		val = registers.read_index<U8>(register_index);
	} else if (is_32_bit_op_inst(operand_size_override)) {
		// 32 bit value
		val = registers.read_index<U32>(register_index);
	} else {
		// 16 bit value
		val = registers.read_index<U16>(register_index);
	}
	return val;
}

U32 CPU::inst_get_address(U32 address_value, bit address_size_override, bit address_byte_size_override) const
{
	U32 val;
	if (address_byte_size_override) {
		// 8 bit value
		val = ram.read<U8>(address_value);
	} else if (is_32_bit_ad_inst(address_size_override)) {
		// 32 bit value
		val = ram.read<U32>(address_value);
	} else {
		// 16 bit value
		val = ram.read<U16>(address_value);
	}
	return val;
}

void CPU::write_to_register(U8 register_index, U32 value, bit operand_size_override, bit operand_byte_size_override)
{
	if (operand_byte_size_override) {
		// 8 bit value
		registers.write_index<U8>(register_index, value);
	}
	else if (is_32_bit_op_inst(operand_size_override)) {
		// 32 bit value
		registers.write_index<U32>(register_index, value);
	}
	else {
		// 16 bit value
		registers.write_index<U16>(register_index, value);
	}
}

void CPU::write_to_memory(U32 address, U32 value, bit address_size_override, bit address_byte_size_override)
{
	if (address_byte_size_override) {
		// 8 bit value
		ram.write<U8>(address, value);
	}
	else if (is_32_bit_ad_inst(address_size_override)) {
		// 32 bit value
		ram.write<U32>(address, value);
	}
	else {
		// 16 bit value
		ram.write<U16>(address, value);
	}
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

void CPU::new_new_execute_instruction()
{
	const Inst_2* inst = static_cast<const Inst_2*>(nullptr); // todo
	currentInstruction = inst;
	
	ALU::branchMonitor.reset();
	
	InstData data = inst->getInstData();
	
	// Read the operands
	U32 op1_val = 0;
	if (inst->read_op1) {
		switch(inst->op1_type) {
		case Inst_2::REG:
			data.op1 = inst_get_operand(inst->op1_register, inst->operand_size_override, inst->operand_byte_size_override);
			data.op1_size = get_size(inst->operand_size_override, inst->operand_byte_size_override);
			break;
		case Inst_2::MEM:
			data.op1 = inst_get_address(inst->address_value, inst->address_size_override, inst->address_byte_size_override);
			data.op1_size = get_size(inst->address_size_override, inst->address_byte_size_override);
			break;
		case Inst_2::IMM:
			data.op1 = inst->immediate_value;
			data.op1_size = get_size(inst->operand_size_override, inst->operand_byte_size_override);	
			break;
		case Inst_2::M_M:
			// used only by BOUND for the second operand
			break;
		case Inst_2::NONE:
			break;
		}
	}
	
	if (inst->read_op2) {
		switch(inst->op2_type) {
		case Inst_2::REG:
			data.op2 = inst_get_operand(inst->op2_register, inst->operand_size_override, inst->operand_byte_size_override);
			data.op2_size = get_size(inst->operand_size_override, inst->operand_byte_size_override);	
			break;
		case Inst_2::MEM:
			data.op2 = inst_get_address(inst->address_value, inst->address_size_override, inst->address_byte_size_override);
			data.op2_size = get_size(inst->address_size_override, inst->address_byte_size_override);	
			break;
		case Inst_2::IMM:
			data.op2 = inst->immediate_value;
			data.op2_size = get_size(inst->operand_size_override, inst->operand_byte_size_override);	
			break;
		case Inst_2::M_M:
			// used only by BOUND
			// the fact that we are addressing memory more than once could be problematic
			data.op2 = inst_get_address(inst->address_value, inst->address_size_override, 0);
			data.op2_size = get_size(inst->address_size_override, 0);	
			if (is_32_bit_ad_inst(inst->address_size_override)) {
				data.op3 = inst_get_address(inst->address_value + 4, inst->address_size_override, 0);
			}
			else {
				data.op3 = inst_get_address(inst->address_value + 2, inst->address_size_override, 0);
			}
			data.op3_size = data.op2_size;
			break;
		case Inst_2::NONE:
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
	if (inst->write_to_dest) {
		switch (inst->op1_type) {
		case Inst_2::REG:
			write_to_register(inst->op1_register, return_value, inst->operand_size_override, inst->operand_byte_size_override);
			break;
		case Inst_2::MEM:
			write_to_memory(inst->address_value, return_value, inst->address_size_override, inst->address_byte_size_override);
			break;
		default:
			break;
		}
	}
	else if (inst->register_out_override) {
		registers.write(inst->register_out, return_value);
	}
}

void CPU::execute_non_arithmetic_instruction(const Inst_2* inst)
{
	
}

void CPU::execute_arithmetic_instruction(const U8 opcode, const InstData data, U32& flags, U32& ret, U32& ret2)
{
	switch (opcode & 0x7F) // consider only the first 7 bits of the opcode
	{
	case Opcodes_2::AAA:
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
	case Opcodes_2::AAD:
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
	case Opcodes_2::AAM:
	{
		U8 q, r;
		bit divByZero;
		ALU::unsigned_divide(U8(data.op1 & 0xFF), (U8)10, q, r, divByZero);
		
		registers.write(ISA::Registers::AH, q);
		registers.write(ISA::Registers::AL, r);

		update_sign_flag(flags, r, OpSize::B);
		update_zero_flag(flags, r);
		update_parity_flag(flags, r);
		
		ret = (q << 8) | r;
		break;
	}
	case Opcodes_2::AAS:
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
	case Opcodes_2::ADC:
	{
		bit carry = flags & Flags::CF;
		ret = ALU::add(data.op1, data.op2, carry);

		update_status_flags(flags, data.op1, data.op2, ret, data.op1_size, data.op2_size, data.op1_size, carry);
		break;
	}
	case Opcodes_2::ADD:
	{
		bit carry = 0;
		ret = ALU::add(data.op1, data.op2, carry);
		
		update_status_flags(flags, data.op1, data.op2, ret, data.op1_size, data.op2_size, data.op1_size, carry);
		break;
	}
	case Opcodes_2::AND:
	{
		ret = ALU::and_(data.op1, data.op2);
		
		flags &= ~(Flags::CF | Flags::OF);
		update_parity_flag(flags, ret);
		update_sign_flag(flags, ret, data.op1_size);
		update_zero_flag(flags, ret);
		break;
	}
	case Opcodes_2::ARPL:
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
	case Opcodes_2::BOUND:
	{
		if (!ALU::compare_greater_or_equal(data.op1, data.op2) || ALU::compare_greater(data.op1, data.op3)) {
			INTERRUPT("#BR", 5);
		}
		break;
	}
	case Opcodes_2::BSF:
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
	case Opcodes_2::BSR:
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
	case Opcodes_2::BT:
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
	case Opcodes_2::BTC:
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
	case Opcodes_2::BTR:
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
	case Opcodes_2::BTS:
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
	case Opcodes_2::CBW:
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
	case Opcodes_2::CLC:
	{
		flags &= ~Flags::CF;
		break;
	}
	case Opcodes_2::CLD:
	{
		flags &= ~Flags::DF;
		break;
	}
	case Opcodes_2::CLI:
	{
		WARNING("Permission check missing, this instruction should sometimes raise an exception"); // TODO
		flags &= ~Flags::IF;
		break;
	}
	case Opcodes_2::CLTS:
	{
		// this instruction needs special treatment
		WARNING("Permission check missing, this instruction should sometimes raise an exception"); // TODO
		registers.control_flag_write_TS(0);
		break;
	}
	case Opcodes_2::CMC:
	{
		flags ^= Flags::CF;
		break;
	}
	case Opcodes_2::CMP:
	{
		bit carry;
		U32 val = ALU::sub(data.op1, data.op2, carry);
		
		update_status_flags(flags, data.op1, data.op2, val, data.op1_size, data.op2_size, data.op1_size, carry);
		break;
	}
	case Opcodes_2::CWD:
	{
		if (data.op2_size == OpSize::DW) {
			if (ALU::check_is_negative(data.op2)) {
				ret = 0xFFFFFFFF;
			}
			else {
				ret = 0;
			}
		}
		else {
			if (ALU::check_is_negative(data.op2)) {
				ret = 0xFFFF;
			}
			else {
				ret = 0;
			}
		}
		break;
	}
	case Opcodes_2::DAA:
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
	case Opcodes_2::DAS:
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
	case Opcodes_2::DEC:
	{
		ret = ALU::sub_no_carry(data.op1, U8(1));
		
		update_overflow_flag(flags, data.op1, U8(-1), ret, data.op1_size, OpSize::B, data.op1_size);
		update_parity_flag(flags, ret);
		update_sign_flag(flags, ret, data.op1_size);
		update_zero_flag(flags, ret);
		update_adjust_flag(flags, data.op1, -1); 
		break;
	}
	// ------------ perfect implementation (I hope) up to here -------------
	case Opcodes_2::DIV:
	{
		U32 r, q, n = data.op1, d = data.op2;
		bit divByZero = false;
		ALU::unsigned_divide(n, d, q, r, divByZero);
		
		if (divByZero) {
			INTERRUPT("#DE", 0);
		}
		
		
		
		
		break;
	}
	/*
	case Opcodes_2::DAA:
	{
		break;
	}
	*/
	}
	
	registers.write_EIP(ALU::add_no_carry(registers.EIP, 1, true));
	
	return ret;
}
/*
void CPU::new_execute_instruction()
{
	const Inst_2* inst = static_cast<const Inst_2*>(nullptr);
	
	ALU::branchMonitor.reset();
	
	U32 op1_val;
	U32 op2_val;
	U32 op3_val;
	
	bit op1_set = 0;
	bit op2_set = 0;
	bit op3_set = 0;
	
	bit write_to_reg = 0;
	bit write_to_ram = 0;
	U8 out_register;
	U32 out_address;
	
	if (inst->is_op1_register) {
		op1_val = inst_get_operand(inst);
		op1_set = 1;
		
		if (inst->write_to_dest) {
			write_to_reg = 1;
			out_register = inst->op1_register;
		}
	}
	
	if (inst->is_next_op_address) {
		switch ((op1_set << 1) | op2_set)
		{
		case 0b00:
			op1_val = inst_get_address(inst);
			op1_set = 1;
			break;
			
		case 0b10:
			op2_val = inst_get_address(inst);
			op2_set = 1;
			break;
		}
		
		if (inst->write_to_dest) {
			write_to_ram = 1;
			out_address = inst->address_value;
		}
	}
	
	if (inst->is_next_op_register) {
		switch ((op1_set << 1) | op2_set)
		{
		case 0b10:
			op2_val = inst_get_operand(inst, true);
			op2_set = 1;
			break;
			
		case 0b11:
			op3_val = inst_get_operand(inst, true);
			op3_set = 1;
			break;
			
		case 0b00:
		default:
			throw BadInstruction("Incorrect operand specification", registers.EIP);
		}
	}
	
	if (inst->is_next_op_immediate) {
		switch ((op1_set << 2) | (op2_set << 1) | op3_set)
		{
			// no size constraints here because I am lazy
		case 0b000:
			op1_val = inst->immediate_value;
			op1_set = 1;
			break;
			
		case 0b100:
			op2_val = inst->immediate_value;
			op2_set = 1;
			break;
			
		case 0b110:
			op3_val = inst->immediate_value;
			op3_set = 1;
			break;
		
		case 0b111:	
		default:
			throw BadInstruction("Incorrect operand specification", registers.EIP);
		}
	}
	
	if (inst->register_out_override) {
		write_to_reg = 1;
		out_register = inst->register_out;
	}
	
	U16 opcode = inst->opcode;
	switch (opcode)
	{
		
	}
}
*/

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
		op1_val = ram.read<U8>(inst->op1);
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
		op2_val = ram.read<U8>(inst->op2);
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
	/*
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
	*/
	case ISA::Opcodes::CALL:
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
	/*
	case ISA::Opcodes::CBW:
	{
		if (is_32_bit_op_inst(inst->op1_size)) {
			U8 al = registers.read(ISA::Registers::AL);
			U16 ax = ALU::sign_extend((U16) al);
			registers.write(ISA::Registers::AX, ax);
		}
		else {
			U16 ax = registers.read(ISA::Registers::AX);
			U32 eax = ALU::sign_extend((U32) ax);
			registers.write(ISA::Registers::EAX, ax);
		}
		break;
	}
	case ISA::Opcodes::CLC:
	{
		registers.flag_write_CF(0);
		break;
	}
	case ISA::Opcodes::CLD:
	{
		registers.flag_write_DF(0);
		break;
	}
	case ISA::Opcodes::CLI:
	{
		registers.flag_write_IF(0);
		break;
	}
	case ISA::Opcodes::CLTS:
	{
		registers.control_flag_write_TS(0);
		break;
	}
	case ISA::Opcodes::CMC:
	{
		registers.flag_write_CF(!registers.flag_read_CF());
		break;
	}
	case ISA::Opcodes::CMP:
	{
		WARNING("CMP can be incorrect because of ALU::sign_extend not taking into account the original operand size.");
		U32 op2_ext;
		if (is_32_bit_op_inst(inst->op2_size)) {
			op2_ext = op2_val;
		} 
		else {
			op2_ext = ALU::sign_extend(op2_val);
		}
		
		bit carry;
		U32 val = ALU::add(op1_val, op2_ext, carry);
		
		update_status_flags(op1_val, op2_val, val, (inst->op1_type == Inst::R ? inst->op1 : -1), carry);
		break;
	}
	*/
	case ISA::Opcodes::CMPS:
	{
		NYI;
	}
	/*
	case ISA::Opcodes::CWD:
	{
		if (is_32_bit_op_inst(inst->op1_size)) {
			if (ALU::check_is_negative(registers.read(ISA::Registers::EAX))) {
				registers.write(ISA::Registers::EDX, 0xFFFFFFFF);
			}
			else {
				registers.write(ISA::Registers::EDX, 0);
			}
		}
		else {
			if (ALU::check_is_negative(registers.read(ISA::Registers::AX))) {
				registers.write(ISA::Registers::DX, 0xFFFF);
			}
			else {
				registers.write(ISA::Registers::DX, 0);
			}
		}
		break;
	}
	*/
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

bit CPU::is_32_bit_op_inst(bit op_prefix, bit D_flag_code_segment) const
{
	return op_prefix ^ D_flag_code_segment;
}

bit CPU::is_32_bit_ad_inst(bit ad_prefix, bit D_flag_code_segment) const
{
	return ad_prefix ^ D_flag_code_segment;
}

void CPU::push_2(U16 value)
{
	U32 esp = registers.read(ISA::Registers::ESP);
	esp = ALU::add_no_carry(esp, (U32) -2);
	registers.write(ISA::Registers::ESP, esp);
	
	stack.push(value);
}

void CPU::push_4(U32 value)
{
	U32 esp = registers.read(ISA::Registers::ESP);
	esp = ALU::add_no_carry(esp, (U32) -4);
	registers.write(ISA::Registers::ESP, esp);
	
	stack.push(value);
}

U16 CPU::pop_2()
{
	U16 val = stack.top();
	stack.pop();
	
	U32 esp = registers.read(ISA::Registers::ESP);
	esp = ALU::add_no_carry(esp, (U32) 2);
	registers.write(ISA::Registers::ESP, esp);
	
	return val;
}

U32 CPU::pop_4()
{
	U32 val = stack.top();
	stack.pop();
	
	U32 esp = registers.read(ISA::Registers::ESP);
	esp = ALU::add_no_carry(esp, (U32) 4);
	registers.write(ISA::Registers::ESP, esp);
	
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
	bit is_op1_neg, is_op2_neg, is_ret_neg;
	
	switch (op1Size) {
	case OpSize::DW:
		is_op1_neg = ALU::check_is_negative(op1, true);
		break;
		
	case OpSize::W:
		is_op1_neg = ALU::check_is_negative(U16(op1), true);
		break;
		
	case OpSize::B:
		is_op1_neg = ALU::check_is_negative(U8(op1), true);
		break;
	}
	
	switch (op2Size) {
	case OpSize::DW:
		is_op2_neg = ALU::check_is_negative(op2, true);
		break;
		
	case OpSize::W:
		is_op2_neg = ALU::check_is_negative(U16(op2), true);
		break;
		
	case OpSize::B:
		is_op2_neg = ALU::check_is_negative(U8(op2), true);
		break;
	}
	
	switch (retSize) {
	case OpSize::DW:
		is_ret_neg = ALU::check_is_negative(ret, true);
		break;
		
	case OpSize::W:
		is_ret_neg = ALU::check_is_negative(U16(ret), true);
		break;
		
	case OpSize::B:
		is_ret_neg = ALU::check_is_negative(U8(ret), true);
		break;
	}
	
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
	bit is_neg = false;
	switch (size) {
	case OpSize::DW:
		is_neg = ALU::check_is_negative(result, true);
		break;
	case OpSize::W:
		is_neg = ALU::check_is_negative(U16(result), true);
		break;
	case OpSize::B:
		is_neg = ALU::check_is_negative(U8(result), true);
		break;
	}
	if (is_neg) {
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
		flags &= ~Flags::FZ;
	}
}

void CPU::update_adjust_flag(U32& flags, U32 op1, U32 op2)
{
	/*
	Adjust flag is set only if there were an carry from the first 4 bits of the AL register to the 4 other bits.
	It is 0 otherwise, including when the operation didn't used the AL register.
	This function should only be called with instructions modifing the AL register (or AX and EAX, but not AH).
	*/
	if (currentInstruction->op1_type == Inst_2::REG && currentInstrcuction->op1_register == 0) {
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
	if (ALU::check_parity(U8(result & 0xFF), true)) {
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
	update_overflow_flag(flags, op1, op2, result, op1Size, pp2Size, retSize);
	update_sign_flag(flags, result, retSize);
	update_zero_flag(flags, result);
	update_adjust_flag(flags, op1, op2);
	update_parity_flag(flags, result);
	update_carry_flag(flags, carry);
}
