
#include <iostream>

#include "../ALU.hpp"
#include "../print_instructions.h"
#include "CPU.h"
#include "opcodes.h"


CPU::CPU(Mem::Memory* memory)
	: memory(memory)
{ }


void CPU::startup()
{
    // TODO: see the startup process in the manual instead

    clock_cycle_count = 0;
    halted = false;

    // Setup CR0
    CR0 control_register = registers.get_CR0();

    // Enable protected mode
    bit protected_ = true;
    control_register.set_val(CR0::PE, protected_);
    registers.set_CR0(control_register);

    // Setup registers
    registers.write(Register::ESP, memory->stack_end);
    registers.write_EIP(memory->text_pos);
}


/**
 * Executes the instructions until the end of the instructions list or the maximum number of cycles is reached.
 */
void CPU::run(size_t max_cycles)
{
	while (!halted) {
		new_clock_cycle();
		std::cout << "Cycle " << clock_cycle_count << "\n";
		try {
			execute_instruction(); // handles the incrementation of the EIP register
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
 * Utility to keep track of how many cycles elapsed
 */
void CPU::new_clock_cycle()
{
	clock_cycle_count++;
	// TODO : reset the branch monitor here
}


/**
 * Returns the size of an operand, using the prefixes of the instruction, as well as the D flag in the current segment.
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
 * Executes the instruction read the address pointed by the EIP.
 * Handles the common part of all instructions, which is operands fetching, flags update, and writing the results to their destination.
 * Actual instruction logic is delegated to CPU::execute_arithmetic_instruction(), CPU::execute_non_arithmetic_instruction() and
 * CPU::execute_non_arithmetic_instruction_with_state_machine().
 */
void CPU::execute_instruction()
{
    const Inst& inst = memory->fetch_instruction(registers.EIP);
	currentInstruction = &inst;

    print_instruction(inst);

    InstData data{};
	
	if (inst.should_compute_address()) {
		data.address = compute_address();
	}
	else {
		data.address = inst.address_value;
	}

	// Segment overrides for the operand and are always set to 32 bits, so they are ignored.
	OpSize operand_size = get_size(inst.operand_size_override, inst.operand_byte_size_override);

    // TODO : missing data.op3 value

	bit immediate_loaded = false;
	
	// Read the operands
	if (inst.op1.read) {
        switch(inst.op1.type) {
		case OpType::REG:
			if (is_special_register(inst.op1.reg)) {
				if (inst.op1.reg <= Register::GS) {
					// All segment registers have a fixed length
					data.op1_size = OpSize::W;
				}
				else {
					// Control registers
					data.op1_size = OpSize::DW;
				}
			}
			else {
				data.op1_size = operand_size;
			}
			
			data.op1 = registers.read(inst.op1.reg, data.op1_size);
			break;

		case OpType::MEM:
			data.op1_size = operand_size;
            data.op1 = memory->read(data.address, data.op1_size);
			break;

		case OpType::IMM:
			data.op1_size = operand_size;
			data.op1 = inst.immediate_value;
			immediate_loaded = true;
			break;

        case OpType::IMM_MEM:
            data.op1_size = operand_size;
            data.op1 = inst.address_value;
			break;
		}
	}

	if (inst.op2.read) {
        switch(inst.op2.type) {
		case OpType::REG:
			if (is_special_register(inst.op2.reg)) {
				if (inst.op2.reg <= Register::GS) {
					// All segment registers have a fixed length
					data.op2_size = OpSize::W;
				}
				else {
					// Control registers
					data.op2_size = OpSize::DW;
				}
			}
			else {
				data.op2_size = operand_size;
			}
			
			data.op2 = registers.read(inst.op2.reg, data.op2_size);
			break;

		case OpType::MEM:
			data.op2_size = operand_size;
			data.op2 = memory->read(data.address, data.op2_size);
			break;

		case OpType::IMM:
			data.op2_size = operand_size;
			data.op2 = inst.immediate_value;
			immediate_loaded = true;
			break;

        case OpType::IMM_MEM:
            data.op2_size = operand_size;
			data.op2 = inst.address_value;
            break;
		}
	}
	
	if (!immediate_loaded) {
		// Load the immediate value only if it has not been stored in an operand already
		data.imm = inst.immediate_value;
	}
	
	// Get the flags registers if needed
	EFLAGS flags;
	if (inst.get_flags) {
	    flags.value = registers.EFLAGS;
	}
	
	// Execute the instruction
	U32 return_value = 0;
	U32 return_value_2 = 0;
	if (inst.opcode & Opcodes::not_arithmetic) {
		// Non-trivial op
		data.op_size = operand_size;

		if (inst.opcode & Opcodes::state_machine) {
			// Include all state machine, jump and string instructions
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

	// Write the output of the instruction to its destination
	if (inst.write_ret1_to_op1) {
		switch (inst.op1.type) {
		case OpType::REG:
			registers.write(inst.op1.reg, return_value);
			break;
		case OpType::MEM:
            memory->write(inst.address_value, return_value, data.op1_size);
			break;
		default:
			break;
		}
	}

    if (inst.write_ret2_to_register) {
        if (inst.scale_output_override) {
            registers.write(inst.register_out, return_value_2, data.op1_size);
        }
        else {
            registers.write(inst.register_out, return_value_2);
        }
    }
    else if (inst.write_ret2_to_op2) {
		switch (inst.op2.type) {
		case OpType::REG:
			registers.write(inst.op2.reg, return_value_2);
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
 * Computes the effective address of the address operand of the current instruction.
 */
U32 CPU::compute_address() const
{
	U32 address = 0;
    if (currentInstruction->displacement_present) {
        address = currentInstruction->address_value;
    }

    U32 index_address = 0;
    if (currentInstruction->reg_present) {
        index_address = registers.read(static_cast<Register>(currentInstruction->reg));

        // Scale the index using chained shifters
        switch (currentInstruction->scale)
        {
            case 0b11: index_address <<= 1; [[fallthrough]]; // *8 NOLINT(bugprone-branch-clone)
            case 0b10: index_address <<= 1; [[fallthrough]]; // *4
            case 0b01: index_address <<= 1; [[fallthrough]]; // *2
            default:   break;							     // *1
        }
    }

    if (currentInstruction->base_present) {
        U32 base = registers.read_index(currentInstruction->base_reg, OpSize::DW);
        index_address = ALU::add_no_carry(index_address, base);
    }

    if (currentInstruction->reg_present || currentInstruction->base_present) {
        U32 index = memory->read(index_address, OpSize::DW);
        address = ALU::add_no_carry(address, index);
    }

	return address;
}


/**
 * Push a value to the stack, of variable size.
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

    memory->write(esp, value, size);
}


/**
 * Pop a value from the stack.
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
 * Called to trigger an interrupt: saves the position in the program, setup the stack and error code data,
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
 * Utility function used to update the value of the adjust flag, after a arithmetic operation using the value of the AL register.
 */
void CPU::update_adjust_flag(EFLAGS& flags, U32 op1, U32 op2)
{
	/*
	Adjust flag is set only if there were a carry from the first 4 bits of the AL register to the 4 other bits.
	It is 0 otherwise, including when the operation didn't use the AL register.
	This function should only be called with instructions modifying the AL register (or AX and EAX, but not AH).
	*/
	if (currentInstruction->op1.type == OpType::REG && currentInstruction->op1_reg_index() == 0) {
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
 * Utility function used to update all arithmetic flags.
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
 * Read bytes from the IO buffer.
 */
U32 CPU::read_io(U8 io_address, OpSize size)
{
    return io.read(io_address, size);
}


/**
 * Write bytes to the IO buffer.
 */
void CPU::write_io(U8 io_address, U32 value, OpSize size)
{
    io.write(io_address, value, size);
}
