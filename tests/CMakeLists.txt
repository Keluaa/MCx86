
add_executable(tests
        ALU_tests.cpp
        RAM_tests.cpp
        program_tests.cpp
        tests_main.cpp)

target_link_libraries(tests mcx86_lib)
