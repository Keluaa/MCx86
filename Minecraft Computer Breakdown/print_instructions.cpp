
#include <iostream>
#include <vector>

#include "CPU/instructions.h"
#include "CPU/opcodes.h"


void print_instruction(const Inst* inst)
{
	std::string opcode;
	
	auto it = Opcodes::mnemonics.find(inst->opcode);
	if (it == Opcodes::mnemonics.cend()) {
		opcode = "???";
	}
	else {
		opcode = it->second;
	}
	
	if (inst->is_op1_none()) {
		
	}
	

}


void print_instructions(const std::vector<Inst>* insts, int start, int count)
{
	for (int i = start; i < start + count; i++) {
		print_instruction(insts[i]);
	}
}
