
cmake_minimum_required(VERSION 3.16) 
project(tmlang)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set( SOURCES src/tm.cpp src/scanner.cpp src/token.cpp src/error.cpp src/parser.cpp )

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories ( ${PROJECT_NAME} PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include )

enable_testing()

file(GLOB  TEST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.tm" )

foreach(TEST_FILE ${TEST_FILES} )
    get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
    add_test ( NAME "test_${TEST_NAME}" COMMAND ${PROJECT_NAME} "${TEST_FILE}")
endforeach()



