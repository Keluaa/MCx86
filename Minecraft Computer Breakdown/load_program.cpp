
#include <string>
#include <fstream>
#include <iostream>
#include <cstdint>
#include <vector>
#include <pair>

#include "instructions.h"

#include "memory/memory_manager.hpp"


std::pair<U8*, U8*>& load_memory_contents(std::filebuf& memory_file, uint32_t rom_start, uint32_t ram_start)
{
	U8* rom = new U8[Mem::ROM_SIZE] { };
	U8* ram = new U8[Mem::RAM_SIZE] { };
	
	// TODO
	
	return std::make_pair(rom, ram);
}


std::vector<const Inst>* load_instructions(std::filebuf& instructions_file, uint32_t inst_start, uint32_t inst_end)
{
    uint32_t instructions_count = inst_end - inst_start;

	auto* instructions = new std::vector<Inst>(instructions_count);

    auto expected_read_count = std::streamsize(instructions_count * sizeof(Inst));
    auto read_count = instructions_file.sgetn(reinterpret_cast<char*>(instructions->data()), expected_read_count);

    if (read_count != expected_read_count) {
        std::cout << "Error while reading instructions data: read only " << read_count << " bytes,"
                  << " expected " << expected_read_count << " bytes.\n";
    }

    return instructions;
}


Mem::Memory* load_memory(const std::string& memory_map_filename,
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
	
	auto& [rom, ram] = load_memory_contents(memory_file, rom_start, ram_start);
	
	memory_file.close();
	
	std::filebuf instructions_file;
	if (!instructions_file.open(instructions_filename, std::ios::in | std::ios::binary)) {
		std::cout << "Could not open the instructions file '" << instructions_filename << "'\n";
		return;
	}
	
	std::vector<const Inst>* instructions = load_instructions(instructions_file, inst_start, inst_end);
	
	instructions_file.close();
	
	Mem::Memory* memory = new Memory(inst_start, instructions.size() * sizeof(Inst),
									 rom_start, rom, ram,
									 instructions);
	
	return memory;
}
