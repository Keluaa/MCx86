
#include <iostream>
#include <iomanip>
#include <vector>

#include "CPU/instructions.h"
#include "CPU/opcodes.h"


std::string optype_to_str(OpType type)
{
	switch (type)
	{
	case OpType::REG:     return "REG";
	case OpType::MEM:     return "MEM";
	case OpType::IMM:     return "IMM";
	case OpType::IMM_MEM: return "IMM_MEM";
	default:              return "???";
	}
}


std::string register_to_str(Register reg)
{
	switch (reg)
	{
	case Register::EAX: return "EAX";
	case Register::ECX: return "ECX";
	case Register::EDX: return "EDX";
	case Register::EBX: return "EBX";
	case Register::ESP: return "ESP";
	case Register::EBP: return "EBP";
	case Register::ESI: return "ESI";
	case Register::EDI: return "EDI";
	case Register::AX:  return "AX";
	case Register::CX:  return "CX";
	case Register::DX:  return "DX";
	case Register::BX:  return "BX";
	case Register::SP:  return "SP";
	case Register::BP:  return "BP";
	case Register::SI:  return "SI";
	case Register::DI:  return "DI";
	case Register::AL:  return "AL";
	case Register::CL:  return "CL";
	case Register::DL:  return "DL";
	case Register::BL:  return "BL";
	case Register::AH:  return "AH";
	case Register::CH:  return "CH";
	case Register::DH:  return "DH";
	case Register::BH:  return "BH";
	case Register::CS:  return "CS";
	case Register::SS:  return "SS";
	case Register::DS:  return "DS";
	case Register::ES:  return "ES";
	case Register::FS:  return "FS";
	case Register::GS:  return "GS";
	case Register::CR0: return "CR0";
	case Register::CR1: return "CR1";
	default:            return "??";
	}
}


void print_operand(const Inst& inst, const Inst::Operand& op)
{
	std::cout << optype_to_str(op.type) << " ";
    switch (op.type)
    {
    case OpType::REG:
        std::cout << register_to_str(op.reg) << " ";
        break;

    case OpType::MEM:
    {
        bool first = true;
        std::cout << "[";
        if (inst.base_reg_present) {
            first = false;
            std::cout << register_to_str(static_cast<Register>(static_cast<U8>(op.reg) & 0b111));
        }
        if (inst.scaled_reg_present) {
            if (!first) std::cout << "+";
            first = false;
            std::cout << register_to_str(static_cast<Register>(inst.scaled_reg))
                      << "*"
                      << (1 << ((static_cast<U8>(op.reg) & 0b11000) >> 3));
        }
        if (inst.address_value != 0) {
            if (!first) std::cout << "+";
            std::cout << "disp";
        }
        std::cout << "] ";
        break;
    }

    default:
        break;
    }
	if (op.read) {
		std::cout << "R ";
	}
	else {
		std::cout << "W ";
	}
}


void print_instruction(U32 inst_address, const Inst& inst)
{
    std::cout << "0x"
              << std::hex << std::setfill('0') << std::setw(5) << inst_address
              << std::dec << std::setfill(' ')
              << "  ";

	std::string opcode;

	auto it = Opcodes::mnemonics.find(inst.opcode);
	if (it == Opcodes::mnemonics.cend()) {
		opcode = "???";
	}
	else {
		opcode = it->second;
	}

	std::cout << " " << std::setw(5) << std::left << opcode << std::right << " ";

	if (inst.operand_byte_size_override) {
		std::cout << "S8 ";
	}
	else if (inst.operand_size_override) {
		std::cout << "S16 ";
	}

	if (inst.is_op1_none()) {
		std::cout << ". ";
	}
	else {
		print_operand(inst, inst.op1);
	}

	if (inst.is_op2_none()) {
		std::cout << ". ";
	}
	else {
		print_operand(inst, inst.op2);
	}

	if (inst.write_ret2_to_register) {
		std::cout << "OUT ";
		if (inst.scale_output_override) {
			std::cout << "s";
		}
		std::cout << register_to_str(inst.register_out) << " ";
	}

	if (inst.address_value != 0) {
		std::cout << "ADDR 0x" << std::hex << inst.address_value << " " << std::dec;
	}
		
	if (inst.immediate_value != 0) {
		std::cout << "IMM 0x" << std::hex << inst.immediate_value << " " << std::dec;
	}

	std::cout << "\n";
}


void print_instructions(const std::vector<Inst>* insts, int start, size_t count, size_t address)
{
	for (int i = start; i < start + count; i++) {
		print_instruction(address, (*insts)[i]);
        address++;
    }
}
