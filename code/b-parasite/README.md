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

# Bluetooth Low Energy Advertisement Data Encoding
Sensor data is encoded in the BLE advertisement packet as Service Data for the [Environmental Sensing Service profile](https://www.bluetooth.com/specifications/assigned-numbers/environmental-sensing-service-characteristics/) (UUID 0x181a).

Sensor data is encoded in unsigned 16 bits (2 bytes), and whenever multiple
 bytes are used to represent a single value, the encoding is big-endian.

| Byte index |                          Description                           |
|------------|----------------------------------------------------------------|
| 0          | 4 bits for protocol version + 4 reserved bits                  |
| 1          | 4 reserved bits + 4 bits wrap-around counter for deduplication |
| 2-3        | Battery voltage in millivolts                                  |
| 4-5        | Temperature in millidegrees Celcius                            |
| 6-7        | Relative air humidity, scaled from 0 (0%) to 0xffff (100%)     |
| 8-9        | Soil moisture, scaled from from 0 (0%) to 0xffff (100%)        |
| 10-15      | b-parasite's own MAC address, big-endian format                |

# Supported Modules

This code supports two E73 modules:
 * E73-2G4M08S1C (nrf2480, default)
 * E73-2G4M08S1E (nrf2833) 

To choose for which one you want to compile, just pass PLATFORM as an env variable to make, and set it to the platform you want to use. For example, to compile for E73-2G4M08S1E:


```bash
SDK_ROOT=<...> PLATFORM=E73_2G4M08S1E make
```

and vice-cerase for E73-2G4M08S1C, although that platform will be chosen as default anyways:


```bash
SDK_ROOT=<...> PLATFORM=E73_2G4M08S1C make
```
