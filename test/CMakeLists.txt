add_executable(example main.cpp)

target_link_libraries(example PRIVATE ${SOURCE})

target_include_directories(example PUBLIC ${CMAKE_SOURCE_DIR}/include)