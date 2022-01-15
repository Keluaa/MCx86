
project(Minecraft_Computer_Breakdown)

set(HEADER_FILES
        ALU.hpp
		data_types.h
		load_program.h
		print_instructions.h
		logger.h
		CPU/CPU.h
		CPU/exceptions.h
		CPU/instructions.h
		CPU/opcodes.h
		CPU/interrupts.h
		CPU/registers.h
		CPU/register_flags_interface.h
		memory/exceptions.hpp
		memory/memory_interfaces.hpp
		memory/memory_manager.hpp
		memory/descriptor_table.hpp
		memory/RAM.hpp
		memory/ROM.hpp
		memory/stack.hpp
		memory/StaticBinaryTreeManagedMemory.hpp
		memory/buffer.hpp)

set(SOURCE_FILES
		load_program.cpp
		print_instructions.cpp
		CPU/CPU.cpp
        CPU/CPU_arithmetic_instructions.cpp
		CPU/CPU_non_arithmetic_instructions.cpp
		CPU/CPU_state_machine_instructions.cpp
		CPU/registers.cpp
		CPU/opcodes.cpp logger.cpp)

add_library(Minecraft_Computer_Breakdown_lib STATIC ${SOURCE_FILES} ${HEADER_FILES})
