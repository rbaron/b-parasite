cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(ble)

include_directories(src)

target_sources(app PRIVATE
	src/main.c
	src/ble.c
	src/encoding.c
)

add_subdirectory(../../prstlib prstlib)
target_include_directories(app PRIVATE ../../prstlib/include)
target_link_libraries(app PUBLIC prstlib)