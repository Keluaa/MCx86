
#include <iostream>

#include "ALU.hpp"
#include "CPU.h"
#include "opcodes.h"


CPU::CPU(Mem::Memory* memory)
	: memory(memory),
      instructions(memory->get_instructions())
{ }


void CPU::switch_protected_mode(bit protected_)
{
	// TODO: see the startup process in the manual instead
	CR0 control_register = registers.get_CR0();

	control_register.set_val(CR0::PE, protected_);

	registers.set_CR0(control_register);
}


/**
 * @brief Executes the instructions until the end of the instructions list or the maximum number of cycles is reached.
 */
void CPU::run(size_t max_cycles)
{
	while (registers.EIP < instructions->size()) {
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
constexpr OpSize CPU::get_size(bit size_override, bit byte_size_override, bit D_flag_code_segment)
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
 * @brief Executes the instruction read the address pointed by the EIP.
 * Handles the common part of all instructions, which is operands fetching, flags update, and writing the results to their destination.
 * Actual instruction logic is delegated to CPU::execute_arithmetic_instruction(), CPU::execute_non_arithmetic_instruction() and
 * CPU::execute_non_arithmetic_instruction_with_state_machine().
 */
void CPU::execute_instruction()
{
	// TODO: I think I misunderstood what the address size override prefix: it only changes the size of the address,
	//  not how many bytes are fetched from memory, which is also controlled by the operand size prefix 
	const Inst& inst = instructions->at(registers.EIP); // TODO: low level instruction fetching
	currentInstruction = &inst;

	InstData data = inst.getInstData();
	
	// compute address using the mod r/m, SIB and displacement bytes
	if (inst.compute_address) {
		data.address = compute_address(!inst.address_size_override, data.op1_size); // TODO: more things to do to have the size of the address
	}

	// TODO : fetch the segment's D bit, maybe also store it and update it when there is a CS change
	OpSize operand_size = get_size(inst.operand_size_override, inst.operand_byte_size_override);
	OpSize address_size = get_size(inst.address_size_override, inst.address_byte_size_override);
	
	// TODO: use the computed address to read the operands. Also fix the operands size.
	
	// Read the operands
	if (inst.read_op1) {
		switch(inst.op1_type) {
		case OpType::REG:
			data.op1_size = operand_size;
			data.op1 = registers.read_index(inst.op1_register, data.op1_size);
			break;
		case OpType::MEM:
			data.op1_size = address_size;
            data.op1 = memory->read(inst.address_value, data.op1_size);
			break;
		case OpType::IMM:
			data.op1_size = operand_size;
			data.op1 = inst.immediate_value;
			break;
		case OpType::M_M:
			// used only by BOUND for the second operand
			break;
		case OpType::SREG:
			data.op1_size = OpSize::W; // all segment registers have a fixed length
			data.op1 = registers.read_segment(inst.op1_register);
			break;
		case OpType::MOFF:
			data.op1_size = operand_size;
			WARNING("moffs operands are not correctly addressed."); // TODO
			data.op1 = memory->read(inst.address_value, data.op1_size);
			break;
		case OpType::CREG:
			data.op1_size = OpSize::DW;
			data.op1 = registers.read_control_register(inst.op1_register);
			break;
		case OpType::NONE:
			break;
		}
	}
	
	if (inst.read_op2) {
		switch(inst.op2_type) {
		case OpType::REG:
			data.op2_size = operand_size;
			data.op2 = registers.read_index(inst.op2_register, data.op2_size);
			break;
		case OpType::MEM:
			data.op2_size = address_size;
			data.op2 = memory->read(inst.address_value, data.op2_size);
			break;
		case OpType::IMM:
			data.op2_size = operand_size;
			data.op2 = inst.immediate_value;
			break;
		case OpType::M_M:
			// used only by BOUND
			// the fact that we are addressing memory more than once could be problematic
			data.op2_size = get_size(inst.address_size_override, 0);
			data.op2 = memory->read(inst.address_value, data.op2_size);
			if (data.op2_size == OpSize::DW) {
				data.op3 = memory->read(inst.address_value + 4, data.op2_size);
			}
			else {
				data.op3 = memory->read(inst.address_value + 2, data.op2_size);
			}
			data.op3_size = data.op2_size;
			break;
		case OpType::SREG:
			data.op2_size = OpSize::W;
			data.op2 = registers.read_segment(inst.op2_register);
			break;
		case OpType::MOFF:
			data.op2_size = operand_size;
			WARNING("moffs operands are not correctly addressed."); // TODO
			data.op2 = memory->read(inst.address_value, data.op1_size);
			break;
		case OpType::CREG:
			data.op2_size = OpSize::DW;
			data.op2 = registers.read_control_register(inst.op2_register);
			break;
		case OpType::NONE:
			break;
		}
	}

	// get the flags registers if needed
	EFLAGS flags;
	if (inst.get_flags) {
	    flags.value = registers.EFLAGS;
	}
	
	// execute the instruction
	U32 return_value = 0;
	U32 return_value_2 = 0;
	if (inst.opcode & Opcodes::not_arithmetic) {
		// non-trivial op
		data.op_size = operand_size;
		data.ad_size = address_size;

		if (inst.opcode & Opcodes::state_machine) {
			// include all state machine, jump and string instructions
			execute_non_arithmetic_instruction_with_state_machine(inst.opcode, data, flags);
		}
		else {
			execute_non_arithmetic_instruction(inst.opcode, data, flags, return_value, return_value_2);
		}
	}
	else {
		execute_arithmetic_instruction(inst.opcode, data, flags, return_value, return_value_2);
	}
	
	if (inst.get_flags) {
		// write the new flags
		registers.EFLAGS = flags.value;
	}

	// write the output of the instruction to its destination
	if (inst.write_ret1_to_register) {
		if (inst.scale_output_override) {
			registers.write_index(inst.register_out, return_value, data.op1_size);
		}
		else {
			// TODO: fix register addressing here, because we can only access 8 registers like this 
			registers.write(inst.register_out, return_value);
		}
	}
	else if (inst.write_ret1_to_op1) {
		switch (inst.op1_type) {
		case OpType::REG:
			registers.write_index(inst.op1_register, return_value, data.op1_size);
			break;
		case OpType::MEM:
            memory->write(inst.address_value, return_value, data.op1_size);
			break;
		default:
			break;
		}
	}
	
	if (inst.write_ret2_to_op2) {
		switch (inst.op2_type) {
		case OpType::REG:
			registers.write_index(inst.op2_register, return_value_2, data.op2_size);
			break;
		case OpType::MEM:
            memory->write(inst.address_value, return_value_2, data.op2_size);
			break;
		default:
			break;
		}
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
			case 0b11: index_val <<= 1; [[fallthrough]]; // *8 NOLINT(bugprone-branch-clone)
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
			// TODO: this switch statement cam greatly be simplified
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

        default:
            throw BadInstruction("Invalid RM byte", registers.EIP); // shouldn't happen
		}
		
		address = ALU::add_no_carry(address, a);
		address = ALU::add_no_carry(address, b);
	}
    
    // TODO : displacement bytes following the Mod r/m byte in both modes (can only be present when mod != 0b11)
    
    // TODO : fetch the address value when mod != 0b11, READ THE DOCS! 
	
	return address;
}


/**
 * @brief Push a value to the stack, of variable size.
 * @param value The value to push
 * @param size The size of the value
 */
void CPU::push(U32 value, OpSize size)
{
    // TODO : move this function to Mem::Stack
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

    memory->get_stack()->write(esp, value, size);
}


/**
 * @brief Pop a value from the stack.
 * @param size The size of the value.
 * @return The value popped
 */
U32 CPU::pop(OpSize size)
{
    // TODO : move this function to Mem::Stack
	U32 esp = registers.read(Register::ESP);

	if (size == OpSize::UNKNOWN) {
		size = get_size(currentInstruction->operand_size_override, 0);
	}

    U32 val = memory->read(esp, size);

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
 * then switch to the corresponding interrupt handler. There is several things we need push to the stack, so we must
 * use the state machine design in order to spread them in several clock cycles.
 *
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
 * @brief Utility function used to update the value of the adjust flag, after a arithmetic operation using the value of the AL register.
 */
void CPU::update_adjust_flag(EFLAGS& flags, U32 op1, U32 op2)
{
	/*
	Adjust flag is set only if there were an carry from the first 4 bits of the AL register to the 4 other bits.
	It is 0 otherwise, including when the operation didn't used the AL register.
	This function should only be called with instructions modifying the AL register (or AX and EAX, but not AH).
	*/
	if (currentInstruction->op1_type == OpType::REG && currentInstruction->op1_register == 0) {
		// Not the implementation used in the circuit, which is much simpler, 
		// as this flag can come out from the adder directly.
		bit AF = (op1 & 0x0F) + (op2 & 0x0F) > 0x0F; // TODO : check operation order with the manual
		flags.set_val(EFLAGS::AF, AF);
	}
	else {
	    flags.clear(EFLAGS::AF);
	}
}


/**
 * @brief Utility function used to update all arithmetic flags
 */
void CPU::update_status_flags(EFLAGS& flags, U32 op1, U32 op2, U32 result, OpSize op1Size, OpSize op2Size, OpSize retSize, bit carry)
{
	// updates all status flags
	flags.update_overflow_flag(op1, op2, result, op1Size, op2Size, retSize);
	flags.update_sign_flag(result, retSize);
	flags.update_zero_flag(result);
	flags.update_parity_flag(result);
    flags.set_val(EFLAGS::CF, carry);
    update_adjust_flag(flags, op1, op2);
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
