# Overview

This is the b-parasite formware based on Nordic's [nRF5 SDK](https://infocenter.nordicsemi.com/index.jsp?topic=%2Fstruct_sdk%2Fstruct%2Fsdk_nrf5_latest.html&cp=7_1).

It uses Nordic's SoftDevice, which should additionally be flashed to the chip before running our firmware.

I use a [JLink probe](https://www.segger.com/products/debug-probes/j-link/) for flashing and debugging.

# Configuration
The b-parasite specific configuration, such as active/sleep time  and transmitting power are defined in [config/prst_config.h](./config/prst_config.h).

# Flashing SoftDevice and Firmware
```bash
# Flash softdevice
$ SDK_ROOT=~/dev/nrf52/sdk/nRF5_SDK_17.0.2_d674dde make flash_softdevice
# Compile annd flash our firmware
$ SDK_ROOT=~/dev/nrf52/sdk/nRF5_SDK_17.0.2_d674dde make flash
```

# Debugging
Calls to `NRF_LOG` will be readable on the console using `JLinkRTTLogger`. This is the handy one-liner I use for pulling log messages:

```bash
$ echo "\n\n\n\n0\n/dev/stdout" | JLinkRTTLogger | sed 's/^.*app: //'
```