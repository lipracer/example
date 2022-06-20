cmake_minimum_required(VERSION 3.15.0)

set(CMAKE_C_COMPILER "/usr/bin/gcc-7")
set(CMAKE_CXX_COMPILER "/usr/bin/g++-7")

project (example)

set(CMAKE_CXX_STANDARD 14)

add_subdirectory(include)
add_subdirectory(lib)
add_subdirectory(test)