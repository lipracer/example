cmake_minimum_required(VERSION 3.12.0)

# IF (CMAKE_SYSTEM_NAME MATCHES "Linux") 
# set(CMAKE_C_COMPILER "/usr/bin/gcc-7")
# set(CMAKE_CXX_COMPILER "/usr/bin/g++-7")
# ENDIF()

project (example)

set(CMAKE_CXX_STANDARD 14)

add_subdirectory(include)
add_subdirectory(lib)
add_subdirectory(test)