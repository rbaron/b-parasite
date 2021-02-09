# My development board
* [E73-TBB(52832)](https://www.ebyte.com/en/product-view-news.aspx?id=889)
* [Manual PDF](file:///Users/rbaron/Downloads/E73-TBX_UserManual_EN_v1.0(1).pdf)
* 512KB flash
* 64KB RAM

* Using the [Generic](https://github.com/sandeepmistry/arduino-nRF5/blob/master/boards.txt#L93) board variant from arduino-nRF5. This is kinda similar to the [Adafruit feather nrf52832 board definition](https://github.com/platformio/platform-nordicnrf52/blob/develop/boards/adafruit_feather_nrf52832.json), but seems to use sandeepmistry:openocd instead of nrfutil and uses a different linker as well.

# Articles
* Great article about using Rust and Apache Mynewt, but also covers J-Link, ST-Link, openocd, unlocking the nrf52. [Link on medium](https://medium.com/@ly.lee/coding-nrf52-with-rust-and-apache-mynewt-on-visual-studio-code-9521bcba6004)
