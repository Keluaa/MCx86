cmake_minimum_required(VERSION 3.17)
project(mcx86)

set(CMAKE_CXX_STANDARD 20)


set(SOURCE_FILES "main.cpp")
add_executable(mcx86_run ${SOURCE_FILES})

include_directories("src")
add_subdirectory("src")

target_link_libraries(mcx86_run mcx86_lib)

add_subdirectory("tests")
add_subdirectory("compare_with_processor")
