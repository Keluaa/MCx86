
#include "doctest.h"

#include <map>
#include <ranges>
#include <iostream>
#include <limits>

#include "CPU/CPU.h"
#include "CPU/instructions.h"
#include "CPU/opcodes.h"
#include "memory/memory_manager.hpp"


template<typename Iter>
Mem::Memory* create_memory(Iter start, Iter end)
{
    std::vector<Inst> instructions_cpy(start, end);

    const U32 text_pos = 0x10000;
    const U32 rom_pos = 0x200000;

    U8* rom = new U8[Mem::ROM_SIZE] { };
    U8* ram = new U8[Mem::RAM_SIZE] { };

    Mem::Memory* memory = new Mem::Memory(text_pos, instructions_cpy.size() * sizeof(Inst), instructions_cpy,
                                          rom_pos, rom, ram);

    return memory;
}


TEST_CASE("EFLAGS")
{
    const std::array instructions_tests{
        std::make_tuple(
            "ADD 2,-7", EFLAGS::SF,
            0x2,
            Inst{
                .opcode = Opcodes::ADD,
                .op1 = {.type = OpType::REG, .reg = Register::EAX, .read = true},
                .op2 = {.type = OpType::IMM, .read = true},
                .get_flags = true,
                .write_ret1_to_op1 = true,
                .immediate_value = static_cast<U32>(-0x7),
            },
            0x2 + static_cast<U32>(-0x7)
        ),
        std::make_tuple(
            "ADD 2, 7", EFLAGS::PF,
            0x2,
            Inst{
                .opcode = Opcodes::ADD,
                .op1 = { .type = OpType::REG, .reg = Register::EAX, .read = true },
                .op2 = { .type = OpType::IMM, .read = true },
                .get_flags = true,
                .write_ret1_to_op1 = true,
                .immediate_value = 0x7,
            },
            static_cast<U32>(0x2 + 0x7)
        ),
        std::make_tuple(
            "ADD -0x7FFF_FFFF, 0xFFF_FFFF", EFLAGS::PF | EFLAGS::AF | EFLAGS::SF,
            -0x7FFFFFFF,
            Inst{
                .opcode = Opcodes::ADD,
                .op1 = { .type = OpType::REG, .reg = Register::EAX, .read = true },
                .op2 = { .type = OpType::IMM, .read = true },
                .get_flags = true,
                .write_ret1_to_op1 = true,
                .immediate_value = 0xFFFFFFF,
            },
            static_cast<U32>(-0x7FFFFFFF) + 0xFFFFFFF
        ),
        std::make_tuple(
            "ADD 0x7FFF_FFFF, -1", EFLAGS::CF | EFLAGS::AF,
            std::numeric_limits<I32>::max(),
            Inst{
                .opcode = Opcodes::ADD,
                .op1 = { .type = OpType::REG, .reg = Register::EAX, .read = true },
                .op2 = { .type = OpType::IMM, .read = true },
                .get_flags = true,
                .write_ret1_to_op1 = true,
                .immediate_value = static_cast<U32>(-1),
            },
            static_cast<U32>(0x7ffffffe)
        ),
        std::make_tuple(
            "SUB 7, 2", EFLAGS::PF,
            0x7,
            Inst{
                .opcode = Opcodes::SUB,
                .op1 = { .type = OpType::REG, .reg = Register::EAX, .read = true },
                .op2 = { .type = OpType::IMM, .read = true },
                .get_flags = true,
                .write_ret1_to_op1 = true,
                .immediate_value = 0x2,
            },
            static_cast<U32>(0x7 - 0x2)
        ),
        std::make_tuple(
            "SUB 2, 7", EFLAGS::CF | EFLAGS::AF | EFLAGS::SF,
            0x2,
            Inst{
                .opcode = Opcodes::SUB,
                .op1 = { .type = OpType::REG, .reg = Register::EAX, .read = true },
                .op2 = { .type = OpType::IMM, .read = true },
                .get_flags = true,
                .write_ret1_to_op1 = true,
                .immediate_value = 0x7,
            },
            static_cast<U32>(0x2 - 0x7)
        ),
        std::make_tuple(
            "SUB -0x7FFF_FFFF, 0xFFF_FFFF", EFLAGS::OF | EFLAGS::AF,
            -0x7FFFFFFF,
            Inst{
                .opcode = Opcodes::SUB,
                .op1 = { .type = OpType::REG, .reg = Register::EAX, .read = true },
                .op2 = { .type = OpType::IMM, .read = true },
                .get_flags = true,
                .write_ret1_to_op1 = true,
                .immediate_value = 0xFFFFFFF,
            },
            static_cast<U32>(0x70000002)
        ),
        std::make_tuple(
            "SUB 0x7FFF_FFFF, -1", EFLAGS::CF | EFLAGS::PF | EFLAGS::SF | EFLAGS::OF,
            std::numeric_limits<I32>::max(),
            Inst{
                .opcode = Opcodes::SUB,
                .op1 = { .type = OpType::REG, .reg = Register::EAX, .read = true },
                .op2 = { .type = OpType::IMM, .read = true },
                .get_flags = true,
                .write_ret1_to_op1 = true,
                .immediate_value = static_cast<U32>(-1),
            },
            static_cast<U32>(std::numeric_limits<I32>::min())
        )
    };

    auto instructions = instructions_tests | std::views::elements<3>;
    Mem::Memory* memory = create_memory(instructions.begin(), instructions.end());
    CPU cpu(memory);

    Registers& registers = cpu.get_registers();
    int i = 0;
    for (const auto& [test_name, expected_flags, first_value, _, expected_result] : instructions_tests) {
        SUBCASE(test_name) {
            cpu.startup();
            registers.EIP = cpu.get_memory().text_pos + i;
            registers.write(Register::EAX, first_value);
            cpu.execute_instruction();

            const U32 result = registers.read(Register::EAX);
            const U32 expected_flags_value = expected_flags | EFLAGS::default_value;

            CHECK_EQ(result, expected_result);
            CHECK_EQ(registers.flags.value, expected_flags_value);

            std::cout << "Got: \t" << registers.flags.print() << std::endl;
            if (registers.flags.value != expected_flags_value) {
                registers.flags.value = expected_flags_value;
                std::cout << "Expected: " << registers.flags.print() << std::endl;
            }
        }
        i++;
    }

    delete memory;
}
