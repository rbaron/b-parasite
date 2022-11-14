cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(b_parasite)

# set(BOARD_ROOT .)

target_sources(app PRIVATE src/main.c src/prst/shtc3.c src/prst/adc.c)
