#pragma once

#include <vector>

#include "CPU/instructions.h"


std::string optype_to_str(OpType type);
std::string register_to_str(Register reg);
void print_instruction(U32 inst_address, const Inst& inst);
void print_instructions(const std::vector<Inst>* insts, int start, size_t count, size_t address);
