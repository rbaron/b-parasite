# SPDX-License-Identifier: Apache-2.0

if (CONFIG_BPARASITE_NRF52833)
board_runner_args(jlink "--device=nRF52833_xxAA" "--speed=4000")
board_runner_args(pyocd "--target=nrf52833" "--frequency=4000000")
elseif (CONFIG_BPARASITE_NRF52840)
board_runner_args(jlink "--device=nRF52840_xxAA" "--speed=4000")
board_runner_args(pyocd "--target=nrf52840" "--frequency=4000000")
endif()

# set(OPENOCD_NRF5_SUBFAMILY "nrf52")
include(${ZEPHYR_BASE}/boards/common/nrfjprog.board.cmake)
include(${ZEPHYR_BASE}/boards/common/jlink.board.cmake)
include(${ZEPHYR_BASE}/boards/common/pyocd.board.cmake)
include(${ZEPHYR_BASE}/boards/common/openocd-nrf5.board.cmake)
include(${ZEPHYR_BASE}/boards/common/blackmagicprobe.board.cmake)