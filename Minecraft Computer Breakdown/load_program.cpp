
#include <string>
#include <fstream>
#include <iostream>
#include <cstdint>
#include <vector>

#include "CPU/instructions.h"

#include "memory/memory_manager.hpp"


std::pair<U8*, U8*> load_memory_contents(std::filebuf& memory_file, uint32_t rom_size, uint32_t ram_size)
{
	U8* rom = new U8[Mem::ROM_SIZE] { };
	U8* ram = new U8[Mem::RAM_SIZE] { };

    memory_file.sgetn(reinterpret_cast<char*>(rom), rom_size);
    memory_file.sgetn(reinterpret_cast<char*>(ram), ram_size);

	return std::make_pair(rom, ram);
}


std::vector<Inst>* load_instructions(std::filebuf& instructions_file, uint32_t inst_start, uint32_t inst_end)
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
		return nullptr;
	}
	
	uint32_t entry_point, 
			 inst_start, inst_end,
			 rom_start,
			 ram_start,
             raw_rom_size,
             raw_ram_size;

    memory_map_file >> std::hex
                    >> entry_point
                    >> inst_start >> inst_end
                    >> rom_start
                    >> ram_start
                    >> raw_rom_size
                    >> raw_ram_size;

	memory_map_file.close();

    if (rom_start + Mem::ROM_SIZE != ram_start) {
        std::cout << "The RAM should start where the ROM ends. "
                  << "ROM start: 0x" << std::hex << rom_start
                  << ", ROM end: 0x" << rom_start + Mem::ROM_SIZE
                  << ", RAM start: 0x" << ram_start << "\n";
        return nullptr;
    }

	std::filebuf memory_file;
	if (!memory_file.open(memory_contents_filename, std::ios::in | std::ios::binary)) {
		std::cout << "Could not open the memory contents file '" << memory_contents_filename << "'\n";
		return nullptr;
	}

	auto&& [rom, ram] = load_memory_contents(memory_file, raw_rom_size, raw_ram_size);
	
	memory_file.close();
	
	std::filebuf instructions_file;
	if (!instructions_file.open(instructions_filename, std::ios::in | std::ios::binary)) {
		std::cout << "Could not open the instructions file '" << instructions_filename << "'\n";
		return nullptr;
	}
	
	std::vector<Inst>* instructions = load_instructions(instructions_file, inst_start, inst_end);

	instructions_file.close();
	
	Mem::Memory* memory = new Mem::Memory(inst_start, instructions->size() * sizeof(Inst),
                                          rom_start, rom, ram,
                                          *instructions);

    std::cout << "Allocated " << static_cast<double>(memory->get_RAM()->get_size()) / 1000000 << " MB of RAM"
              << ", with a "
              << static_cast<double>(Mem::StaticBinaryTreeManagedMemory<Mem::RAM_SIZE, sizeof(U32)>::get_tree_cells_size()) / 1000000
              << " MB allocator tree.\n";

	return memory;
}
