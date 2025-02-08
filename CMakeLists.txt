cmake_minimum_required(VERSION 3.6)

project("ARP_assignments" C CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

add_compile_options(-Wall -Wextra -Wpedantic)

add_subdirectory(include)
add_subdirectory(src)
