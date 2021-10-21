
project(Minecraft_Computer_Breakdown)

set(HEADER_FILES
        ALU.hpp
        circuit_branch_monitor.h
        CPU.h
        data_types.h
        exceptions.h
        IA-32_decoder.h
        IA-32_mapping.h
        instructions.h
        interrupts.h
        registers.h
		memory/exceptions.hpp
		memory/memory_interfaces.hpp
		memory/memory_manager.hpp
		memory/descriptor_table.hpp
		memory/RAM.hpp
		memory/ROM.hpp
		memory/stack.hpp
		memory/StaticBinaryTreeManagedMemory.hpp memory/buffer.hpp)

set(SOURCE_FILES
        CPU.cpp
        CPU_instructions.cpp
        IA-32_mapping.cpp
        instructions.cpp
        registers.cpp
        load_program.cpp 
		CPU_non_arithmetic_instructions.cpp 
		CPU_state_machine_instructions.cpp)

add_library(Minecraft_Computer_Breakdown_lib STATIC ${SOURCE_FILES} ${HEADER_FILES})
