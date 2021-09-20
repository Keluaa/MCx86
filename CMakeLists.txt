cmake_minimum_required(VERSION 3.17)
project(Minecraft_Computer_Breakdown_main)

set(CMAKE_CXX_STANDARD 20)


set(SOURCE_FILES "Minecraft Computer Breakdown.cpp")
add_executable(Minecraft_Computer_Breakdown_run ${SOURCE_FILES})
set_target_properties(Minecraft_Computer_Breakdown_run PROPERTIES WIN32_EXECUTABLE TRUE)

include_directories("Minecraft Computer Breakdown")
add_subdirectory("Minecraft Computer Breakdown")

target_link_libraries(Minecraft_Computer_Breakdown_run Minecraft_Computer_Breakdown_lib)

add_subdirectory("Computer Tests")
