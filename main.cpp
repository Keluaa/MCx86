
#include <iostream>
#include <csignal>

#include "CPU/CPU.h"
#include "load_program.h"
#include "print_instructions.h"


static const char memory_map_filename[] = "../executable_file_data/memory_map.txt";
static const char memory_contents_filename[] = "../executable_file_data/memory_data.bin";
static const char instructions_filename[] = "../executable_file_data/instructions.bin";


/**
 * Basic signal handling to set the exit code as the signal code.
 */
volatile std::sig_atomic_t g_signal_status;
void signal_handler(int sig)
{
    g_signal_status = sig;
    std::quick_exit(sig);
}


void quick_exit_handler()
{
	if (g_signal_status != 0) {
		std::cerr << "Signal: ";
		switch (g_signal_status)
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


void print_program_instructions(Mem::Memory* memory)
{
    std::cout << "Instructions:\n";
    const std::vector<Inst>* insts = memory->get_instructions();
    print_instructions(insts, 0, insts->size(), memory->text_pos);
    std::cout << "\n";
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

    print_program_instructions(memory);

    try {
        CPU cpu(memory);
        cpu.startup();
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
