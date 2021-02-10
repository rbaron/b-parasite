# My development board
* [E73-TBB(52832)](https://www.ebyte.com/en/product-view-news.aspx?id=889)
* [Manual PDF](file:///Users/rbaron/Downloads/E73-TBX_UserManual_EN_v1.0(1).pdf)
* 512KB flash
* 64KB RAM

* Using the [Generic](https://github.com/sandeepmistry/arduino-nRF5/blob/master/boards.txt#L93) board variant from arduino-nRF5. This is kinda similar to the [Adafruit feather nrf52832 board definition](https://github.com/platformio/platform-nordicnrf52/blob/develop/boards/adafruit_feather_nrf52832.json), but seems to use sandeepmistry:openocd instead of nrfutil and uses a different linker as well.

# Articles
* Great article about using Rust and Apache Mynewt, but also covers J-Link, ST-Link, openocd, unlocking the nrf52. [Link on medium](https://medium.com/@ly.lee/coding-nrf52-with-rust-and-apache-mynewt-on-visual-studio-code-9521bcba6004)
*
# nrf command line utilities
* [nrfjprog](https://www.nordicsemi.com/Software-and-tools/Development-Tools/nRF-Command-Line-Tools) Used for programming hex files using the J-Link programmer.
* [nrfutil](https://github.com/NordicSemiconductor/pc-nrfutil) Used for [DFU](https://en.wikipedia.org/wiki/USB#Device_Firmware_Upgrade) - device firmware update - via the USB port. No J-Link. This is what Adafruit uses for uploading code via USB. This is "higher level" and requires a pre-existing bootloader in the nrf52 chip that understands DFU. The [Adafruit_nRF52_Bootloader](https://github.com/adafruit/Adafruit_nRF52_Bootloader) is such a bootloader, which we can burn using nrfjprog + J-Link once. Adafruit has its own version, [`adafruit-nrfutil`](https://github.com/adafruit/Adafruit_nRF52_nrfutil).

# SWD vs. Bootloader vs. DFU
* How `nrfutil` is used in the platformio nordicnrf52 package: [link](https://github.com/platformio/platform-nordicnrf52/blob/develop/builder/main.py#L319)
* Seems like we can use DFU if we install a DFU-enabled bootloader like the [Adafruit_nRF52_Bootloader](https://github.com/adafruit/Adafruit_nRF52_Bootloader)
* Using the adafruit nrf52 bootloader with the E73-TBB [link](https://ssihla.wordpress.com/2019/07/23/using-adafruits-bluefruit-nrf52-feather-bootloader-on-a-cheap-chineese-board/)

## Burning the Adafruit bootloader
1. `$ brew cask install nordic-nrf-command-line-tools` -> Didn't work. I had to manually install the pkgs from [nordic](https://www.nordicsemi.com/Software-and-tools/Development-Tools/nRF-Command-Line-Tools/Download)
1. `pip install intelhex`
1. `PATH=$PATH:/Users/rbaron/.platformio/packages/framework-arduinoadafruitnrf52/tools/adafruit-nrfutil/macos make BOARD=feather_nrf52840_express all`

## Burning our custom bootloader
The error I was making was using a non-softdevice bootloader. The following works:
```bash
# Compile
$ PATH=$PATH:/Users/rbaron/.platformio/packages/framework-arduinoadafruitnrf52/tools/adafruit-nrfutil/macos make BOARD=e73_tbb all

# Flash (make sure you use the _s132_ hex file)
$ JAVA_HOME=/Users/rbaron/homebrew/Cellar/openjdk/15.0.1 nrfjprog --program _build/build-e73_tbb/e73_tbb_bootloader-0.4.0-2-g4ba802d_s132_6.1.1.hex --sectoranduicrerase -f nrf52 --reset
```

Now we can drop the JLink and use the USB serial for uploading/monitoring.

## Uploading with nrfutil
```
$ ~/.platformio/packages/framework-arduinoadafruitnrf52/tools/adafruit-nrfutil/macos/adafruit-nrfutil dfu genpkg --dev-type 0x0052 --sd-req 0x00B7 --application .pio/build/e73-tbb/firmware.hex dfu-pkg.zip
Zip created at dfu-pkg.zip

$ ~/.platformio/packages/framework-arduinoadafruitnrf52/tools/adafruit-nrfutil/macos/adafruit-nrfutil dfu serial --package dfu-pkg.zip -p /dev/cu.usbserial-14330 -b 115200
Upgrading target on /dev/cu.usbserial-14330 with DFU package /Users/rbaron/dev/parasite/code/parasite/dfu-pkg.zip. Flow control is disabled, Dual bank, Touch disabled
########################################
########################################
########################################
########################################
########################################
#######################
Activating new firmware
Device programmed.
```
