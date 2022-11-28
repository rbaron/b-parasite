cmake_minimum_required(VERSION 3.20.0)

# Pull in the dts/ and boards/ from prstlib.
set(DTS_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../prstlib)
set(BOARD_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../prstlib)

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(blinky)

target_sources(app PRIVATE src/main.c)