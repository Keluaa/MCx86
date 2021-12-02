
#include <iostream>

#include "ALU.hpp"
#include "CPU.h"
#include "opcodes.h"


/**
 * @brief  Executes the instruction specified by its opcode. This method handles special instructions which needs to
 * access or modify special data, like stack operations, I/O, descriptor table operations...
 * @param data Holds instruction information
 * @param flags EFLAGS register
 * @param ret Return value of the instruction
 * @param ret2 Additional return value of the instruction
 */
void CPU::execute_non_arithmetic_instruction(const U8 opcode, const InstData data, EFLAGS& flags, U32& ret, U32& ret2)
{
	// TODO : check if ret and ret2 are both needed

	switch (opcode) // we could consider only the first 7 bits of the opcode
	{
	case Opcodes::HLT:
	{
		// TODO ? (here is also a privilege check to do)
		break;
	}
	case Opcodes::IN:
	{
	    const CR0 control_register = registers.get_CR0();

		U8 CPL = 0; // TODO : get CPL from the segment descriptor
		if (control_register.get(CR0::PE) && (flags.get(EFLAGS::VM) || ALU::compare_greater(CPL, flags.read_IOPL()))) {
			// We are in protected mode, with CPL > IOPL or in virtual mode
			// TODO : default IO permission bit value: 0
			if (false) {
                throw_exception(Interrupts::GeneralProtection);
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
		// TODO : use CR0 class instead
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
	    const CR0 control_register = registers.get_CR0();

		U8 CPL = 0; // TODO : get CPL from the segment descriptor
		if (control_register.get(CR0::PE) && (flags.get(EFLAGS::VM) || ALU::compare_greater(CPL, flags.read_IOPL()))) {
			// We are in protected mode, with CPL > IOPL or in virtual mode
			// TODO : default IO permission bit value: 0
			if (false) {
				throw ProcessorException("#GP", registers.EIP, 0);
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
		    flags.value = pop(OpSize::DW);
		}
		else {
			flags.value = (flags.value & 0xFF00) | pop(OpSize::W);
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
			push(flags.value, OpSize::DW);
		}
		else {
			push(flags.value & 0xFFFF, OpSize::W);
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
		// TODO : use the CR0 class instead
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
		// TODO ? (here is also a privilege check to do, I think)
		break;
	}
	default:
	{
		throw UnknownInstruction("Unknown Non-arithmetic instruction", opcode, registers.EIP);
	}
	}

	registers.write_EIP(ALU::add_no_carry(registers.EIP, 1));
}

