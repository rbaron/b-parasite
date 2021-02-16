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

## What does each line in the linker mean?
* [The most commented linker script in the world](https://github.com/theacodes/Winterbloom_Castor_and_Pollux/blob/main/firmware/scripts/samd21g18a.ld)

### Memory layout
The [SoftDevice S132 spec](https://infocenter.nordicsemi.com/index.jsp?topic=%2Fsds_s112%2FSDS%2Fs1xx%2Fmem_usage%2Fmem_resource_map_usage.html&anchor=mem_resource_map_usage) specifies the layout as the soft device begins at addr 0x0 and the application code comes after it, at the address `APP_CODE_BASE`.

# PWM
* [HardwarePWM.cpp](https://github.com/adafruit/Adafruit_nRF52_Arduino/blob/a86fd9f36c88db96f676cead1e836377a37c7b05/cores/nRF5/HardwarePWM.cpp#L112) by Adafruit is a good reference. [Servo.c](https://github.com/adafruit/Adafruit_nRF52_Arduino/blob/4d703b6f38262775863a16a603c12aa43d249f04/libraries/Servo/src/nrf52/Servo.cpp#L154) uses it.
* [nrf52832 pwm spec](https://infocenter.nordicsemi.com/index.jsp?topic=%2Fcom.nordic.infocenter.nrf52832.ps.v1.1%2Fpwm.html)
* [nrf5 SDK PWM reference](https://infocenter.nordicsemi.com/index.jsp?topic=%2Fcom.nordic.infocenter.sdk5.v11.0.0%2Fgroup__nrf__pwm__hal.html) HAL => Hardware Abstraction Layer?

# Analog to digital converter (ADC)
* In nrf52-land, the ADC is called [SSADC](https://infocenter.nordicsemi.com/index.jsp?topic=%2Fcom.nordic.infocenter.sdk5.v15.0.0%2Fgroup__nrf__adc.html)
* Example in the sdk: in `examples/peripheral/saadc/main.c`
* [Docs from Adafruit](https://learn.adafruit.com/bluefruit-nrf52-feather-learning-guide/nrf52-adc). It uses Arduino's `analogRead`.

## Resoultion
The default resolution is 10 bits (1024 values),

## Reference value
 The default reference is an internally supplied 3.6V. It seems like we woulld only be able to read values up to 3.6V. This might be enough if we're using a CR2032 coin cell, but might not if we use a LiPo (4.2V fully charged).

**Question**: what happens if Vcc < 3.6?

There is a possibility of using Vcc as a reference. Then I imagine 0 -> 0V; 1024 -> Vcc. This might be exactly what we want:
In a PWM positive pulse, our RC_parasitic will rise to 0.63Vcc in R*C_para time. If we use Vcc as a reference for the ADC, I think we "cancel out" the Vcc factor.

* How adafruit uses the Vcc reference: [link](https://github.com/adafruit/Adafruit_nRF52_Arduino/blob/master/cores/nRF5/wiring_analog_nRF52.c#L76)
* How arduino-nRF5 uses the Vcc refenrece: [link](https://github.com/sandeepmistry/arduino-nRF5/blob/master/cores/nRF5/wiring_analog_nRF52.c#L98)

So in Arduino land, using AR_VDD4 does all we need: use nrf's SAADC_CH_CONFIG_REFSEL_VDD1_4 (Vcc/4 ref) and a gain of 4 (SAADC_CH_CONFIG_GAIN_Gain1_4).
While connected via USB, with:
```
analogReference(AR_VDD4);
int sens_val = analogRead(kSensAnalogPin);
```
I'm getting ~680 when in the air; ~65 while holding the sensor. The default resolution is 10 bits, so 1024 -> 3.3V. On the scope, I'm reading the sensor output value to be 2.16V. This matches the value I'm reading: 1024/3.3 * 2.15 = 667. Nice.

# Battery
* [CR2032 datasheet](https://data.energizer.com/pdfs/cr2032.pdf)
  * 235 mAh (from 3.0V to 2.0V)
  * Typical discharge current: 0.19 mA -> 1200 hours

# Battery monitoring
* Good post on how to measure lipo batteries: [link](https://devzone.nordicsemi.com/nordic/nordic-blog/b/blog/posts/measuring-lithium-battery-voltage-with-nrf52#:~:text=To%20reduce%20the%20leakage%20current,of%2040%20us%2C%20see%20here.)
We have a choice of using different references when using the analog-to-digital converter. For measuing the peak of the RC charging circuit, it makes sense to use VCC as the reference, as the rise in the RC is proportional to VCC.
For battery monitoring, we need an absolute reference. Luckily, we can use the internal reference of 0.6V. To increase the range of values, we can combine this with a gain parameter. This is what the arduino-nrf5 does in [wiring_analog_nRF52.c](https://github.com/sandeepmistry/arduino-nRF5/blob/master/cores/nRF5/wiring_analog_nRF52.c#L92).
With a gain of 1/2, we could read the absolute range of [0, 1.2V]. Since we're interested in the max value of roughly 4.2 (fully charged LiPo, or alternatively 3.0V for a CR2032 coin cell), we can use a voltage divider with R1 = 1470 kOhm and R2 = 470kOhm. This would give us a range of [0, ~5V].
This seems to be working okay, but I need to investigate if making it stiffer (lower R1 and R2) improves the accuracy. With higher resistor values, we minimize the quiescent current, but increase the source impedance. Even hooking up the oscilloscope changes the reading value.

## Ideas for improvement:
* Decrease the impedance of the voltage divider, but somehow use a mcu-controlled switch so we don't pay the current price when the MCU is sleeping (which is most of the time). [This stackexchange answer](https://electronics.stackexchange.com/a/64491) mentions a similar approach
* Use even larger resistor values and attach a capacitor across R2, as suggested by [this answer](https://www.eevblog.com/forum/projects/battery-monitoring-voltage-divider/msg2524116/#msg2524116). We can reach nanoamps of current, which is negligible in this design. Question: does the capacitor self leak? If so, we'd need to be constantly pumping charges into it. How significant is this effect? If we do oversampling, the capacitor probably won't charge in time for multiple fast measurements. But the capacitor might act like a filter itself, negating the need for oversampling.
  * [This post on jeelabs.org](https://jeelabs.org/wp-content/uploads/2013/05/16/measuring-the-battery-without-draining-it/) also does some experiments with a capacitor across R2.

# BLE
* BLE examples for platformio with nordicnrf52 platform: [link](https://github.com/platformio/platform-nordicnrf52/tree/master/examples)
* [nrf5 BLE examples](https://devzone.nordicsemi.com/nordic/short-range-guides/b/bluetooth-low-energy/posts/ble-advertising-a-beginners-tutorial) are way more complicated than I need for now
* How the alternative firmware for the xiaomi temp sensor works: [link](https://github.com/atc1441/ATC_MiThermometer)
  * The MCU is a [Telink TLSR8251](http://wiki.telink-semi.cn/doc/ds/DS_TLSR8251-E_Datasheet%20for%20Telink%20BLE+IEEE802.15.4%20Multi-Standard%20Wireless%20SoC%20TLSR8251.pdf)
  * Deep sleep current is 1-2uA
  * It sends an _advertisement packet_ every 1 minute with the MAC, temperature, humidity and battery level
  * [main](https://github.com/atc1441/ATC_MiThermometer/blob/master/ATC_Thermometer/main.c)
  * [main_loop](https://github.com/atc1441/ATC_MiThermometer/blob/916cef7db24977ec187e68ab6e718b7b7a4988e6/ATC_Thermometer/app.c#L76)
  * [advertisement_data](https://github.com/atc1441/ATC_MiThermometer/blob/master/ATC_Thermometer/ble.c#L39) definition
  * [set_adv_data](https://github.com/atc1441/ATC_MiThermometer/blob/master/ATC_Thermometer/ble.c#L178) - where the temp, humidity and batt levels are encoded into the advertisement data
  * They don't use manufacturer data. They use a custom UUID service (0x181a).
  * Using the manufacturer data requires a SIG membership for getting the manufacturer ID (first two bytes in the manufacturer data)
  * Using the service data also requires applying for a 16-bit (2 byte) ID from SIG ([link](https://www.bluetooth.com/specifications/assigned-numbers/))
  * Aha! I think we can use the 0x181a service UUID (the same one the alternative xiaomi firmware uses). This service is reserved for "Environmental Sensors". Check out the offical assigned numbers list [here](https://btprodspecificationrefs.blob.core.windows.net/assigned-values/16-bit%20UUID%20Numbers%20Document.pdf).
    * We can use the SERVICE_DATA (int 16) in the BLE advertisement packet to pack service data for the UUID 0x181a.
* How the xiaomi BLE temp sensor works in ESPHome: [link](https://github.com/esphome/esphome/blob/5c86f332b269fd3e4bffcbdf3359a021419effdd/esphome/components/xiaomi_lywsd03mmc/xiaomi_lywsd03mmc.cpp)
* [How advertisement works YouTube video](https://www.youtube.com/watch?v=CJcLabp42b4) by nordic
  * In your case:
    * GAP role: broadcaster
    * Advertisement type: legacy ADV_NONCONN_IND (non-connectable broadcast)
    * Advertising data starts at 22:17
      * The payload might contain 37 bytes in the classic/legacy protocol:
        * addr (6 bytes) (either public or random)
          * Public has privacy concerns
          * Random can be static or private (resolvable or non-resolvable)
        * data (31 bytes)
    * The 31 bytes of advertisement data can be split into structures
      * Each structure has a length (1 byte), a type (n bytes) and data (remaining bytes)
      * Examples of structures are service UUID, local name, manufacturer specific data
      * Manufacturer specific data is usually the way to transmit data in the advertisement packet
        * Requires a company id. Check out the [list](https://www.bluetooth.com/specifications/assigned-numbers/company-identifiers). Adafruit has the 0x0822.
* [BLEPeripheral](https://github.com/sandeepmistry/arduino-BLEPeripheral) seems to be a popular library choice
  * [setManufacturerData](https://github.com/sandeepmistry/arduino-BLEPeripheral/blob/161a4163f565be3cd5b62bbc59f0c2b522d82b02/src/BLEPeripheral.h#L72) is probably what we want
  * It seems to be highly geared towards nrf51 SOCs, although it mentions support for some nrf52 ones
* [Adafruit_nRF52_Arduino](https://github.com/adafruit/Adafruit_nRF52_Arduino)
  * [BLEAdvertising guide](https://learn.adafruit.com/bluefruit-nrf52-feather-learning-guide/bleadvertising)
    * [BLEAdvertising.h](https://github.com/adafruit/Adafruit_nRF52_Arduino/blob/master/libraries/Bluefruit52Lib/src/BLEAdvertising.h#L87)
* Can we update the manufaturer advertising data dynamically?
  * [Hint on devzone.nordicsemi.com](https://devzone.nordicsemi.com/f/nordic-q-a/11217/adc-values-in-advertising-data-dynamically-changing)
* What MAC address to use?
  * Public - might have some security implications, since BLE can be used for tracking
  * Random - safer, since its random, but it "breaks" our MAC-addr based Hub config

## Packing values into the advertising packet
### Soil humidity
If there's room, I'd like to pack the raw analog-to-digital value and a percentage.
* Values for the ADC are 10 bit (0-1023). So we could use 10 bits for this. For simplicity, I'll keep this byte-aligned and use 16 bits (2 bytes) for this. It means
  I'll have to scale [0, 2^10) to [0-2^16) (and back in the hub).
* Percentage values are floating points, but we don't have nearly as much precision for justifying 32/64 bits. Even using integers in [0-100] would be fine. I'll splurge and use two bytes here as well. It means I'll have to scale [0, 1.0] to [0, 2^16) and back in the hub.

# Central BLE (hub)
The "central" BLE will be responsible for listening to parasite's BLE broadcasts and parsing its manufacturer's data.
One idea is to use ESPHome for this, for example like the xiaomi sensor does:
* [Xiaomi component's parse_device](https://github.com/esphome/esphome/blob/dev/esphome/components/xiaomi_lywsd03mmc/xiaomi_lywsd03mmc.cpp#L19)
* [esp32_ble_tracker calls all registered parse_devices()](https://github.com/esphome/esphome/blob/dev/esphome/components/esp32_ble_tracker/esp32_ble_tracker.cpp#L68)
* [Setting up the development environment for ESPHome](https://esphome.io/guides/contributing.html#setting-up-development-environment)

Question: parasite will advertise a _lot_ of packets in short bursts. How is this data "deduplicated"? I imagine ESPHome won't parse all the packets and forward them to MQTT.

# OTA

# Measuring current consumption
* Good [issue](https://github.com/atc1441/ATC_MiThermometer/issues/134) on the xiaomi sensor tracker

# Data persistence
* Adafruit has a internal filesystem implementation. See [example](https://github.com/adafruit/Adafruit_nRF52_Arduino/blob/master/libraries/InternalFileSytem/examples/Internal_ReadWrite/Internal_ReadWrite.ino). Here's the [library implementation](https://github.com/adafruit/Adafruit_nRF52_Arduino/tree/master/libraries/InternalFileSytem/src/flash)
* Adafruit's [LittleFS](https://github.com/adafruit/Adafruit_nRF52_Arduino/tree/master/libraries/Adafruit_LittleFS)
* Examples of using [fstorage](https://infocenter.nordicsemi.com/index.jsp?topic=%2Fcom.nordic.infocenter.sdk5.v15.0.0%2Flib_fstorage.html) from nordic: [link](https://github.com/NordicPlayground/nRF5-flash-storage-examples).
* Example from the SDK: `flashwrite` in examples\peripheral\flashwrite
* [sd_flash_write](https://infocenter.nordicsemi.com/index.jsp?topic=%2Fcom.nordic.infocenter.s132.api.v3.0.0%2Fgroup___n_r_f___s_o_c___f_u_n_c_t_i_o_n_s.html)

Questions:
* What addresses are safe to write to? We need to avoid:
  * Bootloader
  * SoftDevice
  * Application
  * More sections?

# Deep sleep
* [Adafruit_nRF52_Arduino](https://github.com/adafruit/Adafruit_nRF52_Arduino/blob/master/cores/nRF5/wiring.h#L34) has a waitForEvent function, which internally calls [sd_app_evt_wait](https://github.com/adafruit/Adafruit_nRF52_Arduino/blob/master/cores/nRF5/wiring.c#L101).
* System ON/OFF. System ON mode is a power saving mode in which the RTCounters are active, so they can emit events for waking up the CPU. System off has no RTC active.
* the RTC COMPARE event can be used to wake up. See the examples/peripheral/rtc.
* [app_timer on nordicsemi](https://devzone.nordicsemi.com/f/nordic-q-a/46031/need-a-30-second-interrupt---using-rtc)
* [Adafruit_nRF52_Arduino issue](https://github.com/adafruit/Adafruit_nRF52_Arduino/issues/165)
  * `delay()` calls waitForEvent?
  * [suspendLoop](https://github.com/adafruit/Adafruit_nRF52_Arduino/blob/master/libraries/Bluefruit52Lib/examples/Peripheral/beacon/beacon.ino#L60) can probably save energy when no deep sleep is used.
  * [More on power consumption](https://github.com/adafruit/Adafruit_nRF52_Arduino/issues/165#issuecomment-407288522). where @hathach mentions that the `delay()` actually uses [`waitForEvent`](https://github.com/adafruit/Adafruit_nRF52_Arduino/blob/4d703b6f38262775863a16a603c12aa43d249f04/cores/nRF5/wiring.c#L84), which itself calls
* [Loopless / low power examples](https://github.com/adafruit/Adafruit_nRF52_Arduino/pull/262) pull request on Adafruit_nRF52_Arduino
  * They use the SoftwareTimer, which

* The example in examples/peripheral/rtc is straight-forward: initialize RTC and set up the TICK and COMPARE0 event. The TICK event is triggered on every clock cycle. The COMPARE0 only when the counter reaches a specified value.
  * It uses the nrf_drv_rtc.h header, which doesn't seem to be included in the Arduino core implementaton
  * There's a similar one in Arduino called [nrfx/hal/nrf_rtc.h](https://github.com/adafruit/Adafruit_nRF52_Arduino/blob/4d703b6f38262775863a16a603c12aa43d249f04/cores/nRF5/nordic/nrfx/hal/nrf_rtc.h). It seems to use the same call to `nrf_rtc_cc_set` under the hood.
  * [nrfx](https://github.com/NordicSemiconductor/nrfx) is "a standalone set of drivers for peripherals present in Nordic Semiconductor's SoCs".

* The [RTC driver](https://infocenter.nordicsemi.com/index.jsp?topic=%2Fcom.nordic.infocenter.sdk5.v15.0.0%2Fgroup__nrfx.html) has some interesting functions:
  * nrfx_rtc_cc_set
  * nrfx_rtc_tick_disable
* Interrupt handlers are registered in the NVIC (nested vectored interrupt controller)

*[wolfssl](https://fossies.org/linux/wolfssl/wolfcrypt/src/port/arm/cryptoCell.c) uses the functions I'm thinking of using.

* Reminders:
  * RTC0 and RTC1 might be in use. Try using RTC2
  * Initialize the low frequency clock LFCLKSRC
  * Disable the TICK interrupt
  * Set up an IRQ (callback) [example from nordic forum](https://devzone.nordicsemi.com/f/nordic-q-a/52923/rtc-interrupt-handler-not-working)
  * Set up the COMPARE[0] event/interrupt
  * Disable LFCLKSTARTED interrupt? [link from nordic forum](https://devzone.nordicsemi.com/f/nordic-q-a/70091/early-wake-up-from-power-clock-irq-when-using-rtc-to-wake-up)
  * FPU (floating point interrupts) also might trigger the interrupt. Important thread from [nordic](https://devzone.nordicsemi.com/f/nordic-q-a/23242/single-float-division-causing-7x-higher-current-draw)
  * Doesn't work with C++?? [devzone](https://devzone.nordicsemi.com/f/nordic-q-a/13465/nvic-registered-interrupt-doesn-t-work)

## Deep Sleep with SoftwareTimer
This is a FreeRTOS scheduled task. It seems to work, but by default it wasn't. I had to increase the FreeRTOS timer stack size, as hinted in [this GitHub issue](https://github.com/adafruit/Adafruit_nRF52_Arduino/issues/188)], in ~/.platformio/packages/framework-arduinoadafruitnrf52/cores/nRF5/freertos/config/FreeRTOSConfig.h.

* Issues
  * nrfx_clock_init is not linked by default in platformio. Is there a more conventional way of using this?
  * freeRTOS on Adafruit_nRF52_Arduino uses the RTC. [link](https://github.com/adafruit/Adafruit_nRF52_Arduino/blob/4d703b6f38262775863a16a603c12aa43d249f04/cores/nRF5/freertos/config/FreeRTOSConfig.h)
# Arduino specifics
* How `main` works by calling `setup` and `loop`. [Link for Adafruit](https://github.com/adafruit/Adafruit_nRF52_Arduino/blob/4d703b6f38262775863a16a603c12aa43d249f04/cores/nRF5/main.cpp#L74), [link for arduino-nrf5](https://github.com/sandeepmistry/arduino-nRF5/blob/master/cores/nRF5/main.cpp#L27). Or: why dropping the `setup` and `loop` doesn't work directly, as they do with ESP32.

## FreeRTOS
* [Low Power Support or tickless idle](https://www.freertos.org/low-power-tickless-rtos.html)
* [Adafruit use of freeRTOS for scheduling the loop() function in Arduino](https://github.com/adafruit/Adafruit_nRF52_Arduino/blob/4d703b6f38262775863a16a603c12aa43d249f04/cores/nRF5/main.cpp#L94)
* [Adafruit's SoftwareTimer](https://github.com/adafruit/Adafruit_nRF52_Arduino/blob/4d703b6f38262775863a16a603c12aa43d249f04/cores/nRF5/utility/SoftwareTimer.cpp) also uses freeRTOS tasks
* How/where is freeRTOS implemented for nrf52?
  * [Adafruit_nRF52_Arduino/cores/nRF5/freertos/](https://github.com/adafruit/Adafruit_nRF52_Arduino/tree/4d703b6f38262775863a16a603c12aa43d249f04/cores/nRF5/freertos)
* [FreeRTOS support in nordic SDK](https://infocenter.nordicsemi.com/index.jsp?topic=%2Fcom.nordic.infocenter.sdk5.v15.0.0%2Fnrf_freertos_example.html)
  * examples/blinky_freertos
  * examples/blinky_rtc_freertos

# Nordic specifics
* HAL vs. Drivers
  * [Great answer on nordic forum](https://devzone.nordicsemi.com/f/nordic-q-a/5964/nrf_dvr_xxx-vs-nrf_xxx)
  * HAL are lower-level. They provide some functions for accessing registers. Drivers are higher level, and usually use the HAL functions. Drivers can be used with or without SoftDevices.


# nRF52840
I've started using a standalone nrf52840 module (e73-2g4m08s1c), since I want a barebones module to test power consumption. This chip is very similar to the nrf52832, except it has built-in usb support!

## Adafruit bootloader
The bootloader is different. It has one extra feature: double resetting puts the chip into DFU/CDC mode. I actually verified that it shows up as a mass storage device on my mac!

## Questions
* Booting in DFU/CDC creates a new serial device in /dev/cu.usbmodem*. What about when the application is running? How does Serial.print() work? How can I read those?
  * framework-arduinoadafruitnrf52/cores/nRF5/TinyUSB/Adafruit_TinyUSB_ArduinoCore/Adafruit_USBD_CDC.h likely has the answer and it uses TinyUSB.
* Reset button seems to be "configurable" to use pin 0.18. Do I need to pre-configure it or will resetting work with Adafruit's bootloader for putting it into flash mode by quickly resetting it twice?
  * In the [ItsyBitsy schematic](https://cdn-learn.adafruit.com/assets/assets/000/087/158/original/adafruit_products_schem.png?1579387035), it seems like the reset switch simply pulls P0.18 to gnd.
  *
## Power input
  * When connected to USB, VBUS is 5V
  * The datasheet mentions 1.7V - 5.5V supply voltage
  * But there are two supply pins (page 79):
    * VDD handles 1.7 - 3.6V
    * VDDH handles 2.5 - 5.5V
  * So, when connecting to USB, do I need to bridge VBUS and VDDH high?
    * (page 65): "As a consequence, VBUS and either VDDH or VDD supplies are required for USB peripheral operation"


## Bluetooth
I got basic sketches working on the barebones e73c breakout, except the ones running bluetooth.
I was able to deviry that the program hands exactly when it reaches the Bluefruit.begin() function.
Googling suggests that it's missing a low frequency oscillating crystal.

Details on oscillator on page 84.

### How to enable the internal synthetic one?
In [arduino-nRF5](https://github.com/sandeepmistry/arduino-nRF5/blob/master/boards.txt#L49) there is a menu in which we can select:
- Crystal (-DUSE_LFXO)
- RC (-DUSE_LFRC)
- Synthetic (-DUSE_LFSYNT)

Changing the following on framework-arduinoadafruitnrf52/variants/feather_nrf52840_express/variant.h made it work:
```C
// #define USE_LFXO      // Board uses 32khz crystal for LF
#define USE_LFRC    // Board uses RC for LF
```

