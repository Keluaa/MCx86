
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
        memory_manager.hpp
        RAM.hpp
        registers.h
        ROM.h
        stack.h
        StaticBinaryTreeManagedMemory.hpp)

set(SOURCE_FILES
        CPU.cpp
        IA-32_mapping.cpp
        instructions.cpp
        registers.cpp)

add_library(Minecraft_Computer_Breakdown_lib STATIC ${SOURCE_FILES} ${HEADER_FILES})
