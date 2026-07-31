
cmake_minimum_required(VERSION 3.16) 
project(tmlang)

set( SOURCES src/tm.cpp src/scanner.cpp src/token.cpp src/error.cpp )

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories ( ${PROJECT_NAME} PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include )

enable_testing()

file(GLOB  TEST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.tm" )

foreach(TEST_FILE ${TEST_FILES} )
    get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
    add_test ( NAME "test_${TEST_NAME}" COMMAND tmlang ${TEST_FILE})
endforeach()

