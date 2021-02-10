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

# Linking
In [platformio-core/tools/pioplatform.py](https://github.com/platformio/platformio-core/blob/9c20ab81cb68f1ffb7a8cac22ce95c4c797643ec/platformio/builder/tools/pioplatform.py#L130):
```Python
if "build.ldscript" in board_config:
    env.Replace(LDSCRIPT_PATH=board_config.get("build.ldscript"))
```
In [platformio-core/platformio.py](https://github.com/platformio/platformio-core/blob/9c20ab81cb68f1ffb7a8cac22ce95c4c797643ec/platformio/builder/tools/platformio.py#L65):
```Python
# append into the beginning a main LD script
if env.get("LDSCRIPT_PATH") and not any("-Wl,-T" in f for f in env["LINKFLAGS"]):
    env.Prepend(LINKFLAGS=["-T", env.subst("$LDSCRIPT_PATH")])
```
The `-T` flag expects a path to a linker script, as per [`ld` docs](https://ftp.gnu.org/old-gnu/Manuals/ld-2.9.1/html_chapter/ld_3.html).

Frameworks, such as the `nordicnrf52`, contain [boards definitions](https://github.com/platformio/platform-nordicnrf52/blob/develop/boards/adafruit_feather_nrf52832.json#L4) that specify the `ldscript`.

1. For the [adafruit_feather_nrf52832](https://github.com/platformio/platform-nordicnrf52/blob/develop/boards/adafruit_feather_nrf52832.json#L4) board, we have:

```
"arduino":{
  "ldscript": "nrf52832_s132_v6.ld"
},
```
Which is defined [here](https://github.com/adafruit/Adafruit_nRF52_Arduino/blob/master/cores/nRF5/linker/nrf52832_s132_v6.ld).

2. For the [nrf52_dk](https://github.com/platformio/platform-nordicnrf52/blob/develop/boards/nrf52_dk.json#L4), we have:
```
"arduino":{
  "ldscript": "nrf52_xxaa.ld"
},
```
Which is the most common linker script, and is part of the nordic official SDK, it seems, available [here](https://github.com/NordicSemiconductor/nrfx/blob/master/mdk/nrf52_xxaa.ld). For reference, check out the linker for the [Adafruit nRF52 Bootloader](https://github.com/adafruit/Adafruit_nRF52_Bootloader/blob/master/linker/nrf52.ld). Here you can see the [memory map](https://learn.adafruit.com/bluefruit-nrf52-feather-learning-guide/hathach-memory-map) of the nRF52 Feather.

## Same platform, different framework
Both the Feather nRF52832 and the Generic boards above point to the same platform, the [nordicnrf52](). Inside this platorm, there's a switch for selecting which framework to use, based on the board name, [here](https://github.com/platformio/platform-nordicnrf52/blob/develop/platform.py#L38):
```Python
if self.board_config(board).get("build.bsp.name",
                                            "nrf5") == "adafruit":
                self.frameworks["arduino"][
                    "package"] = "framework-arduinoadafruitnrf52"
```

### Memory layout
The [SoftDevice S132 spec](https://infocenter.nordicsemi.com/index.jsp?topic=%2Fsds_s112%2FSDS%2Fs1xx%2Fmem_usage%2Fmem_resource_map_usage.html&anchor=mem_resource_map_usage) specifies the layout as the soft device begins at addr 0x0 and the application code comes after it, at the address `APP_CODE_BASE`.