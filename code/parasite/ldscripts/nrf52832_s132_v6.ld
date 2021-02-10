/* Linker script to configure memory regions. */
/* Source: https://raw.githubusercontent.com/adafruit/Adafruit_nRF52_Arduino/master/cores/nRF5/linker/nrf52832_s132_v6.ld */

SEARCH_DIR(.)
GROUP(-lgcc -lc -lnosys)

MEMORY
{
  FLASH (rx)     : ORIGIN = 0x26000, LENGTH = 0x6D000 - 0x26000

  /* SRAM required by S132 depend on
   * - Attribute Table Size
   * - Vendor UUID count
   * - Max ATT MTU
   * - Concurrent connection peripheral + central + secure links
   * - Event Len, HVN queue, Write CMD queue
   */
  RAM (rwx) :  ORIGIN = 0x20003600, LENGTH = 0x20010000 - 0x20003600
}

SECTIONS
{
  . = ALIGN(4);
  .svc_data :
  {
    PROVIDE(__start_svc_data = .);
    KEEP(*(.svc_data))
    PROVIDE(__stop_svc_data = .);
  } > RAM

  .fs_data :
  {
    PROVIDE(__start_fs_data = .);
    KEEP(*(.fs_data))
    PROVIDE(__stop_fs_data = .);
  } > RAM
} INSERT AFTER .data;

/* TODO(rbaron): This should be failing but it's not. Maybe it's loading it from somewhere else? */
INCLUDE "nrf52_common.ld"