
#include <iostream>
#include <bitset>
 
#include "data_types.h"
#include "CPU.h"

#ifdef _MSC_VER
// use a lambda as an alternative for designated initializers
// see https://stackoverflow.com/a/49572324/8662187
#define with_new(T, param, ...) ([&]{ T ${param}; __VA_ARGS__; return new T($); }())
#else
// Should only be enabled when the compiler supports designated initializers
#define $  // clear the $ sign used for the 'with_new' in the other case
#define with_new(T, param, ...) new T{param, __VA_ARGS__} // standard definition
#endif

const Inst** load_fibbonacci(U32& instructions_count)
{
	instructions_count = 6;
	Inst** instructions = new Inst*[instructions_count];
	std::memset(instructions, NULL, instructions_count);

	int i = 0;
	instructions[i++] = with_new(Inst, ISA::Opcodes::MOV, $.op1_type = Inst::R, $.op1 = ISA::Registers::EAX, $.op2_type = Inst::I, $.op2 = 0);
	instructions[i++] = with_new(Inst, ISA::Opcodes::MOV, $.op1_type = Inst::R, $.op1 = ISA::Registers::EDX, $.op2_type = Inst::I, $.op2 = 1);
	instructions[i++] = with_new(Inst, ISA::Opcodes::ADD, $.op1_type = Inst::R, $.op1 = ISA::Registers::EAX, $.op2_type = Inst::R, $.op2 = ISA::Registers::EDX);
	instructions[i++] = with_new(Inst, ISA::Opcodes::XCHG, $.op1_type = Inst::R, $.op1 = ISA::Registers::EAX, $.op2_type = Inst::R, $.op2 = ISA::Registers::EDX);
	instructions[i++] = with_new(Inst, ISA::Opcodes::JNO, $.displacement = -2);
	instructions[i++] = with_new(Inst, ISA::Opcodes::STOP);

	return (const Inst**) instructions;
}

int main()
{
	U32 instructions_count;
	const Inst** instructions = load_fibbonacci(instructions_count);
	
	CPU cpu(instructions, instructions_count);
	cpu.run();
}
