cmake_minimum_required(VERSION 3.16) 
project(tmlang)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE SRC CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/*.hpp"
    "${CMAKE_SOURCE_DIR}/include/*.h"
    "${CMAKE_SOURCE_DIR}/include/*.hpp"
    "${CMAKE_SOURCE_DIR}/libs/source/*.cpp"
    "${CMAKE_SOURCE_DIR}/libs/source/*.h"
    "${CMAKE_SOURCE_DIR}/libs/source/*.hpp"
)

add_executable(${PROJECT_NAME} ${SRC})

# Header files of libraries
target_include_directories(${PROJECT_NAME} PRIVATE
    # "${CMAKE_SOURCE_DIR}/libs/source"
    "${CMAKE_SOURCE_DIR}/include"
)

# Prebuilt libraries to link
file(GLOB PREBUILT_LIBS CONFIGURE_DEPENDS
)
target_link_libraries(${PROJECT_NAME} PRIVATE ${PREBUILT_LIBS})

# Precompiled dynamic library dlls
file(GLOB ALL_DLLS CONFIGURE_DEPENDS
)

if(ALL_DLLS)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${ALL_DLLS}
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
        COMMAND_EXPAND_LISTS
        VERBATIM
    )
endif()

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND "$<$<CONFIG:Release>:${CMAKE_COMMAND};-E;copy_directory_if_different;${CMAKE_SOURCE_DIR}/assets;$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets>"
    COMMAND_EXPAND_LISTS
    VERBATIM
)
