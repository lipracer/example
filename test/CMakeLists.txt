add_executable(example main.cpp)
target_link_libraries(example PRIVATE ${SOURCE})
target_include_directories(example PUBLIC ${CMAKE_SOURCE_DIR}/include)

add_executable(Test Test.cpp FunctorTest.cpp)
target_link_libraries(Test PRIVATE ${SOURCE})
target_include_directories(Test PUBLIC ${CMAKE_SOURCE_DIR}/include)