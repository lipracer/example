
set(UTILS ${CMAKE_BINARY_DIR}/utils/libUTILS.a)
set(SOURCE ${CMAKE_BINARY_DIR}/lib/libSOURCE.a)

add_executable(example main.cpp)
target_link_libraries(example PRIVATE ${SOURCE})
target_include_directories(example PUBLIC ${CMAKE_SOURCE_DIR}/include)

add_executable(Test Test.cpp FunctorTest.cpp GraphTest.cpp)
target_link_libraries(Test PRIVATE ${SOURCE})
target_link_libraries(Test PRIVATE ${UTILS})
target_link_libraries(Test PRIVATE pthread)
target_include_directories(Test PUBLIC ${CMAKE_SOURCE_DIR}/include)