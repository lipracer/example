set(LIBRARY_OUTPUT_PATH  ${CMAKE_BINARY_DIR}/utils)

add_library(UTILS ProcessBarrier.cpp)
target_include_directories(UTILS PUBLIC ${CMAKE_SOURCE_DIR}/include)