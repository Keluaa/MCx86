
#include <filebuf>
#include <string>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <cstdint>

#include "memory/memory_manager.hpp"


void load_memory_contents(std::filebuf& memory_file, uint32_t rom_start, uint32_t ram_start)
{
	
}


void load_instructions(std::filebuf& instructions_file, uint32_t inst_start, uint32_t inst_end)
{
	
}


void load_program(const std::string& memory_map_filename,
				  const std::string& memory_contents_filename,
				  const std::string& instructions_filename)
{
	std::ifstream memory_map_file(memory_map_filename);
	if (!memory_map_file) {
		std::cout << "Could not open the memory map file '" << memory_map_filename << "'\n";
		return;
	}
	
	uint32_t entry_point, 
			 inst_start, inst_end,
			 rom_start,
			 ram_start;
	
	memory_map_file >> std::hex
					>> entry_point
					>> inst_start >> inst_end
					>> rom_start
					>> ram_start;
	
	memory_map_file.close();

	std::filebuf memory_file;
	if (!memory_file.open(memory_contents_filename, std::ios::in | std::ios::binary)) {
		std::cout << "Could not open the memory contents file '" << memory_contents_filename << "'\n";
		return;
	}
	
	load_memory_contents(memory_file, rom_start, ram_start);
	
	memory_file.close();
	
	std::filebuf instructions_file;
	if (!instructions_file.open(instructions_filename, std::ios::in | std::ios::binary)) {
		std::cout << "Could not open the instructions file '" << instructions_filename << "'\n";
		return;
	}
	
	load_instructions(instructions_file, rom_start, ram_start);
	
	instructions_file.close();
}
