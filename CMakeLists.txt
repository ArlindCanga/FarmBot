cmake_minimum_required(VERSION 3.20)
project(FarmBot)

set(CMAKE_GENERATOR_PLATFORM Win32)
set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE SOURCES
    "src/*.cpp"
    "src/*.h"
)

add_library(FarmBot SHARED ${SOURCES} "src/features/MetinFarm.h")

target_include_directories(FarmBot PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(FarmBot PRIVATE
    comctl32
    dwmapi
)

add_custom_command(TARGET FarmBot POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/config.json
        $<TARGET_FILE_DIR:FarmBot>/config.json
)