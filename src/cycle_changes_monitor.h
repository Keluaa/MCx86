#pragma once

#include <array>
#include <utility>
#include <iostream>

#include "data_types.h"
#include "logger.h"
#include "CPU/registers.h"


class ChangesMonitor
{
    ChangesMonitor() noexcept
            : registers_it(registers.begin())
            , memory_it(memory.begin())
    { }

public:
    static ChangesMonitor& get() {
        static ChangesMonitor instance;
        return instance;
    }

	std::array<Register, 32> registers{};
	std::array<std::pair<U32, OpSize>, 8> memory;

    std::array<Register, 32>::iterator registers_it;
    std::array<std::pair<U32, OpSize>, 8>::iterator memory_it;

	void new_clock_cycle()
	{
        registers_it = registers.begin();
		memory_it = memory.begin();
	}
	
	void add_register(Register reg)
	{
        if (registers_it == registers.end()) {
            std::cout << "ERROR\nToo many register changes.\n";
            return;
        }

        *registers_it = reg;
        registers_it++;
	}
	
	void add_memory(U32 address, OpSize size)
	{
        if (memory_it == memory.end()) {
            std::cout << "ERROR\nToo many memory changes.\n";
            return;
        }

        *memory_it = { address, size };
        memory_it++;
	}
};


inline void register_change(Register reg)
{
    if (Logger::get_mode() == Logger::Mode::MONITOR_CHANGES) {
        ChangesMonitor::get().add_register(reg);
    }
}


inline void memory_change(U32 address, OpSize size)
{
    if (Logger::get_mode() == Logger::Mode::MONITOR_CHANGES) {
        ChangesMonitor::get().add_memory(address, size);
    }
}
