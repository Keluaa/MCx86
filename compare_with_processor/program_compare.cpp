
#include <iostream>
#include <fstream>
#include <csignal>
#include <vector>

#include "CPU/CPU.h"
#include "CPU/opcodes.h"
#include "memory/memory_manager.hpp"
#include "cycle_changes_monitor.h"
#include "load_program.h"


static const char memory_map_filename[] = "../../executable_file_data/memory_map.txt";
static const char memory_contents_filename[] = "../../executable_file_data/memory_data.bin";
static const char instructions_filename[] = "../../executable_file_data/instructions.bin";
static const char instructions_map_filename[] = "../../executable_file_data/instructions_map.txt";


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
		std::cout << "SIGNAL\n";
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
		std::cout << std::endl;
	}
}


void load_instructions_map(std::map<U32, U32>& instructions_map)
{
    std::ifstream instructions_map_file(instructions_map_filename);

    instructions_map_file >> std::hex;
    while (instructions_map_file) {
        U32 address, index;
        instructions_map_file >> address;
        instructions_map_file.ignore(1, ',');
        instructions_map_file >> index;

        if (!instructions_map_file) {
            break;
        }

        instructions_map[index] = address;
    }
}


U32 opsize_to_size(OpSize size)
{
    switch (size) {
    case OpSize::DW:      return 4;
    case OpSize::W:       return 2;
    case OpSize::B:       return 1;
    case OpSize::UNKNOWN: return 0;
    }
}


void print_changes(CPU& cpu)
{
    static ChangesMonitor& changes_monitor = ChangesMonitor::get();

    std::cout << "CHANGES\nREG\n";
    for (auto it = changes_monitor.registers.begin(); it != changes_monitor.registers_it; it++) {
        std::cout << Registers::register_to_string(*it) << ",";
    }

    std::cout << "\nMEM\n" << std::hex;
    for (auto it = changes_monitor.memory.begin(); it != changes_monitor.memory_it; it++) {
        std::cout << it->first << ":" << opsize_to_size(it->second) << ",";
    }
    std::cout << std::dec << "\n";
}


void run(CPU& cpu, U32 max_cycles, std::map<U32, U32>& instructions_map)
{
    static ChangesMonitor& changes_monitor = ChangesMonitor::get();

    U32 address, inst_index;
	while (!cpu.is_halted()) {
		cpu.new_clock_cycle();
        changes_monitor.new_clock_cycle();

        inst_index = cpu.get_registers().EIP;
        address = instructions_map[inst_index];
        std::cout << "INST\n" << std::hex << address << std::dec << "\n";

        if (address != 0 && cpu.get_memory().fetch_instruction(inst_index).opcode == Opcodes::INT) {
            // TODO : here we assume that a INT is always a syscall to terminate the program. More checks are needed to be sure of that.
            break;
        }

		try {
			cpu.execute_instruction();
		}
		catch (ExceptionWithMsg& e) {
			std::cout << "ERROR\n";
			std::cout << e.what() << "\n";
			break;
		}

        print_changes(cpu);
	
		if (cpu.get_clock_cycle() >= max_cycles) {
			std::cout << "ERROR\nMAX_CYCLES\n";
			break;
		}
	}
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
        std::cout << "ERROR\n";
        std::cout << e.what() << "\n";
        return EXIT_FAILURE;
    }

    std::map<U32, U32> instructions_map;
    load_instructions_map(instructions_map);

    Logger::set_mode(Logger::Mode::MONITOR_CHANGES);

    std::cout << "OK\n";

    try {
        CPU cpu(memory);
        cpu.startup();
        run(cpu, 1000, instructions_map);
    }
    catch (const std::exception& e) {
        std::cout << "ERROR\n";
        std::cout << e.what() << "\n";
        delete memory;
        return EXIT_FAILURE;
    }

    std::cout << "END\n";

	delete memory;
    return EXIT_SUCCESS;
}
