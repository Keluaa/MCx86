
add_executable(Computer_tests_run
        ALU_tests.cpp
        RAM_tests.cpp
        program_tests.cpp
        tests_main.cpp)

set_target_properties(Computer_tests_run PROPERTIES WIN32_EXECUTABLE TRUE)

target_link_libraries(Computer_tests_run Minecraft_Computer_Breakdown_lib)
