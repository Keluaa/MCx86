
#include <iostream>
#include <bitset>
 
#include "data_types.h"
#include "CPU.h"


/*
Voir pour compiler en flat: on voudrait quelque chose
qui ne se charge pas dans la ram, sans descriptors etc...
Par contre cela implique peut-etre de compiler les ressources
separement, pour les mettre ensuite dans la ram.
Sinon on peut aussi creer des alias pour les descripteurs
de code & data, qui sont separe des autre segments et charges
dans la rom. Le selecteur de segments va alors se diriger vers
la rom.

On veut un raw binairy, pour faire ca, voir:
 - https://stackoverflow.com/q/3615392/8662187
 - ELF file
 - objdump (unix)
 - dumpbin (Windows)
 - le linker me fait tres peur
 - position independant executable
 - static linking obligatoire
 - mais pas pour les libairies du systeme, la il va falloir du dynamic linking (arg non)

Voir Bochs, un projet similaire d'emulation de processeur x86
-> peut-être pas adapté à ce que je veux faire

Voir celui-ci (et ses références):
https://uob-hpc.github.io/SimEng/


Il me FAUT un os? Voir:
https://wiki.osdev.org/Main_Page
Je pleure.

Instruction encoder/decoder
https://intelxed.github.io/ref-manual/index.html

Autre emu intel:
Intel SDE

Description complète des ELF: https://www.youtube.com/watch?v=nC1U1LJQL8o


Bon site de référence : https://sandpile.org/


*/

#ifdef _MSC_VER
// use a lambda as an alternative for designated initializers
// see https://stackoverflow.com/a/49572324/8662187
#define with_new(T, param, ...) ([&]{ T ${param}; __VA_ARGS__; return new T($); }())
#else
// Should only be enabled when the compiler supports designated initializers
#define $  // clear the $ sign used for the 'with_new' in the other case
#define with_new(T, param, ...) new T{param, __VA_ARGS__} // standard definition
#endif

/*
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
*/

/*
const Inst** load_fibbonacci(U32& instructions_count)
{
	instructions_count = 0;
	Inst** instructions = new Inst*[instructions_count];
	std::memset(instructions, 0, instructions_count);

	// TODO

	return (const Inst**) instructions;
}
*/

				 		 
static const char memory_map_filename[] = "executable_file_data/memory_map.txt";
static const char memory_contents_filename[] = "executable_file_data/memory_data.bin";
static const char instructions_filename[] = "executable_file_data/instructions.bin";


int main()
{
	Mem::Memory* memory = load_memory(memory_map_filename, 
									  memory_contents_filename,
									  instructions_filename);
	
	CPU cpu(memory);
	cpu.run();
	
	delete memory;
}
