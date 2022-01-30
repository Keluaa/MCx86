#pragma once


Mem::Memory* load_memory(const std::string& memory_map_filename,
                         const std::string& memory_contents_filename,
                         const std::string& instructions_filename);
