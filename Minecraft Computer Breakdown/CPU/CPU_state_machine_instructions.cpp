
#include "../ALU.hpp"
#include "CPU.h"
#include "opcodes.h"


/**
 * @brief Handles more complex instructions, which may require several clock cycles to execute, or modifies the instruction pointer explicitly.
 * @param data Holds instruction information
 * @param flags EFLAGS register
 */
void CPU::execute_non_arithmetic_instruction_with_state_machine(const U8 opcode, const InstData data, EFLAGS& flags, U32& ret)
{
	// All parameters are stored on pseudo registers which are read
	// each loop. Their values cannot change during execution.
	// When the instruction is completed, 'stop' is set and all
	// data stored in the instruction should be reset.
	// Each instruction should be fought as a state machine, with
	// the index as the main parameter.
	// This design allows for instructions which last several cycles,
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
        // Near call, relative or absolute, based on the displacement.
        // The target address is pre-computed by the transassembler, so there is no distinction between relative or absolute call.
		U32 eip = registers.read_EIP();
		push(eip, OpSize::DW);
		registers.write_EIP(data.address);
		break;
	}
	case Opcodes::INT:
	case Opcodes::IRET:
	case Opcodes::JMP:
	case Opcodes::LEAVE:
	case Opcodes::LOOP:
	case Opcodes::REP:
        throw_NYI("INT, IRET, JMP, LEAVE, LOOP, REP are not yet implemented");
        break; // TODO : all instructions above

    case Opcodes::RET:
    {
        // Near RET only
        U32 eip = pop(OpSize::DW);
        registers.write_EIP(eip);
        if (ALU::check_different_than_zero(data.op1)) {
            // RET with additional popping
            ret = ALU::add_no_carry(data.op1, data.op1);
        }
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
					push(memory->read(storage.enter.ebp, data.op_size), data.op_size);
				}
				else {
					storage.enter.ebp = ALU::sub_no_carry(storage.enter.ebp, 2);
					push(memory->read(storage.enter.ebp, data.op_size), data.op_size);
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

		default:
		    break; // should not happen
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
			jump = !flags.get(EFLAGS::ZF | EFLAGS::CF);
			break;

		case 0b00001: // Above or equal | Not below | Not carry
			// CF = 0
            jump = !flags.get(EFLAGS::CF);
			break;

		case 0b00010: // Below | Carry | Not above or equal
			// CF = 1
            jump = flags.get(EFLAGS::CF);
			break;

		case 0b00011: // Below or equal | Not above
			// ZF = 1 || CF = 1
			jump = flags.get(EFLAGS::ZF | EFLAGS::CF);
			break;

		case 0b00100: // Equal | Zero
			// ZF = 1
            jump = flags.get(EFLAGS::ZF);
			break;

		case 0b00101: // Greater | Not less or equal
			// ZF = 0 || (SF == OF)
			jump = !flags.get(EFLAGS::ZF) || !(flags.get(EFLAGS::SF) ^ flags.get(EFLAGS::OF));
			break;

		case 0b00110: // Greater or Equal | Not less
			// SF == OF
            jump = !(flags.get(EFLAGS::SF) ^ flags.get(EFLAGS::OF));
			break;

		case 0b00111: // Less | Not greater or equal
			// SF != OF
            jump = flags.get(EFLAGS::SF) ^ flags.get(EFLAGS::OF);
			break;

		case 0b01000: // Less or equal | Not greater
			// ZF = 1 && SF != OF
            jump = flags.get(EFLAGS::ZF) && (flags.get(EFLAGS::SF) ^ flags.get(EFLAGS::OF));
			break;

		case 0b01001: // Not equal | Not zero
			// ZF = 0
			jump = !flags.get(EFLAGS::ZF);
			break;

		case 0b01010: // Not overflow
			// OF = 0
			jump = !flags.get(EFLAGS::OF);
			break;

		case 0b01011: // Not parity | Parity odd
			// PF = 0
			jump = !flags.get(EFLAGS::PF);
			break;

		case 0b01100: // Not sign
			// SF = 0
			jump = !flags.get(EFLAGS::SF);
			break;

		case 0b01101: // Overflow
			// OF = 1
			jump = flags.get(EFLAGS::OF);
			break;

		case 0b01110: // Parity | Parity even
			// PF = 1
			jump = flags.get(EFLAGS::PF);
			break;

		case 0b01111: // Sign
			// SF = 1
			jump = flags.get(EFLAGS::SF);
			break;

		case 0b10000: // CX register is zero
			// CX = 0
			jump = ALU::check_equal_zero(U16(registers.read(Register::CX)));
			break;

		case 0b10001: // ECX register is zero
			// ECX = 0
			jump = ALU::check_equal_zero(registers.read(Register::ECX));
			break;

        default:
            throw BadInstruction("Invalid Jump Type", registers.EIP);
		}

		if (jump) {
            registers.write_EIP(data.op1);
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
			registers.write(static_cast<Register>(reversed_index), val);
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
			storage.pusha.esp = registers.read_index(static_cast<U8>(Register::ESP), data.op_size);
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
		registers.write_EIP(ALU::add_no_carry(registers.EIP, 1));
	}
}
