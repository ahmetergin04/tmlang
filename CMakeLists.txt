
cmake_minimum_required(VERSION 3.16) 
project(tmlang)

set( SOURCES src/tm.cpp src/scanner.cpp src/token.cpp src/error.cpp )

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories ( ${PROJECT_NAME} PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include )

enable_testing()

add_test ( NAME run_test COMMAND tmlang ${CMAKE_CURRENT_SOURCE_DIR}/tests/test.tm )
