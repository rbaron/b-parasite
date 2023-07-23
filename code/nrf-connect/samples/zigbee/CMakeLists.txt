cmake_minimum_required(VERSION 3.20.0)

# Pull in the dts/ and boards/ from prstlib.
set(DTS_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../prstlib)
set(BOARD_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../prstlib)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project("b-parasite Zigbee sample")

include_directories(src)

target_sources(app PRIVATE
  src/main.c
  src/debug_counters.c
  src/double_reset_detector.c
  src/factory_reset.c
  src/flash_fs.c
  src/prst_zb_attrs.c
  src/prst_zb_soil_moisture_defs.c
  src/restart_handler.c
  src/watchdog.c
)

add_subdirectory(../../prstlib prstlib)
target_include_directories(app PRIVATE ../../prstlib/include)
target_link_libraries(app PUBLIC prstlib)
