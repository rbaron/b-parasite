cmake_minimum_required(VERSION 3.20.0)

# Pull in the dts/ and boards/ from prstlib.
set(DTS_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../prstlib)
set(BOARD_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../prstlib)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(input)

target_sources(app PRIVATE src/main.c)

add_subdirectory(../../prstlib prstlib)
target_include_directories(app PRIVATE ../../prstlib/include)
target_link_libraries(app PUBLIC prstlib)
