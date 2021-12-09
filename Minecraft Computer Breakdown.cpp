
#include <iostream>
#include <csignal>

#include "CPU/CPU.h"
#include "load_program.h"
#include "print_instructions.h"


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

static const char memory_map_filename[] = "../executable_file_data/memory_map.txt";
static const char memory_contents_filename[] = "../executable_file_data/memory_data.bin";
static const char instructions_filename[] = "../executable_file_data/instructions.bin";


/**
 * Basic signal handling to set the exit code as the signal code.
 */
volatile std::sig_atomic_t gSignalStatus;
void signal_handler(int sig)
{
	gSignalStatus = sig;
    std::quick_exit(sig);
}


void quick_exit_handler()
{
	if (gSignalStatus != 0) {
		std::cerr << "Signal: ";
		switch (gSignalStatus)
		{
		case SIGSEGV: std::cerr << "SIGSEGV"; break;
		case SIGABRT: std::cerr << "SIGABRT"; break;
		case SIGTERM: std::cerr << "SIGTERM"; break;
		case SIGILL:  std::cerr << "SIGILL";  break;
		case SIGFPE:  std::cerr << "SIGFPE";  break;
		case SIGINT:  std::cerr << "SIGINT";  break;
		default:      std::cerr << "unknown"; break;
		}
		std::cerr << "\n";
	}
	
	std::cerr << "Program quick exited." << std::endl;
}


void exit_handler()
{
	std::cerr << "Program exited.";
}


int main()
{
    signal(SIGSEGV, signal_handler);
    signal(SIGABRT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGILL, signal_handler);
    signal(SIGFPE, signal_handler);
    signal(SIGINT, signal_handler);
    
    std::at_quick_exit(quick_exit_handler);
    std::atexit(exit_handler);

    Mem::Memory* memory;

    try {
        memory = load_memory(memory_map_filename,
                             memory_contents_filename,
                             instructions_filename);
    }
    catch (const std::exception& e) {
        std::cout << "Program loading failed.\n";
        std::cerr << e.what() << "\n";
        return EXIT_FAILURE;
    }

    std::cout << "Program loaded.\n";

    std::cout << "Instructions:\n";
    const std::vector<Inst>* insts = memory->get_instructions();
    print_instructions(insts, 0, insts->size());
    std::cout << "\n";

    try {
        CPU cpu(memory);
        cpu.run(1000);
    }
    catch (const std::exception& e) {
        std::cout << "Program failed.\n";
        std::cerr << e.what() << "\n";
        delete memory;
        return EXIT_FAILURE;
    }

    std::cout << "Program finished without errors.\n";

	delete memory;
    return EXIT_SUCCESS;
}
