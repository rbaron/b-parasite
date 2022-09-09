[![b-parasite firmware build](https://github.com/rbaron/b-parasite/actions/workflows/b-parasite.yml/badge.svg?branch=main)](https://github.com/rbaron/b-parasite/actions/workflows/b-parasite.yml)

# b-parasite
<p align="center">
  <img src="img/resized/img1.jpg" width="512px" border="0" alt="PCB front and back photo" />
</p>

b-parasite is an open source Bluetooth Low Energy (BLE) soil moisture and ambient temperature/humidity/light sensor.

# Features
* Soil moisture sensor. I wrote about how capacitive soil moisture sensors works on [this Twitter thread](https://twitter.com/rbaron_/status/1367182806368071685), based on [this great post](https://wemakethings.net/2012/09/26/capacitance_measurement/) on wemakethings.net
* Air temperature and humidity sensor using a [Sensirion's SHTC3](https://www.sensirion.com/en/environmental-sensors/humidity-sensors/digital-humidity-sensor-shtc3-our-new-standard-for-consumer-electronics/)
* Light sensor using an [ALS-PT19](https://everlighteurope.com/ambient-light-sensors/7/ALSPT19315CL177TR8.html) phototransistor
* Powered by a common CR2032 coin cell, with a battery life of possibly over a year - see "Battery Life" below
* Open hardware and open source design

# Wiki Pages
* [Hardware Versions](https://github.com/rbaron/b-parasite/wiki/Hardware-Versions)
* [How to order: PCB fabrication and SMT assembly](https://github.com/rbaron/b-parasite/wiki/How-to-order:-PCB-fabrication-and-SMT-assembly)
* [How to Program](https://github.com/rbaron/b-parasite/wiki/How-to-Program)

# Repository Organization
* [code/b-parasite/](./code/b-parasite/) - firmware code based on the [nRF5 SDK](https://infocenter.nordicsemi.com/index.jsp?topic=%2Fstruct_sdk%2Fstruct%2Fsdk_nrf5_latest.html&cp=7_1)
* [kicad/](./kicad/) - KiCad schematic, layout and fabrication files for the printed circuit board (PCB)
* [data/](data/) - data for testing and sensor calibration
* [bridge/](bridge/) - an [ESPHome](https://github.com/esphome/esphome)-based BLE-MQTT bridge
* [case/](case/) - a 3D printable case

# How It Works
<p align="center">
  <img src="img/excalidraw/diagram.png" border="0" alt="Diagram containing two b-parasites, a bridge & an MQTT broker" />
</p>

b-parasite works by periodically measuring the soil moisture, air temperature/humidity and broadcasting those values via Bluetooth Low Energy (BLE) advertisement packets. After doing so, the board goes into a sleep mode until it's time for another measurement. The sleep interval is configurable - I often use 10 minutes between readings, which is a good compromise between fresh data and saving battery.

At this point, b-parasite's job is done. We have many possibilities of how to capture its BLE advertisement packet and what to do with the data. A common pattern is having a BLE-MQTT bridge that listens for these BLE broadcasts, decodes them and ships the sensor values through MQTT messages. The MQTT broker is then responsible for relaying the sensor data to interested parties. This is the topology shown in the diagram above.

## Integrations
### ESPHome
A popular choice for a BLE-MQTT bridge is the [ESPHome](https://github.com/esphome/esphome) project, which runs on our beloved [ESP32](https://www.espressif.com/en/products/socs/esp32) boards. b-parasite is officially supported and documentation for using it can be found in [the b-parasite ESPHome docs](https://esphome.io/components/sensor/b_parasite.html). An example of using this platform is also available in this repo, under [bridge/](bridge/) (check out [README.md](bridge/README.md) there for more info).

ESPHome is a battle-tested project with a vibrant community, and is currently the most mature b-parasite bridge. ESP32 are also cheap, so you can sprinkle a few of them around the house to cover a wide range, and even share the same ESP32 with other sensors.

### Home Assistant
b-parasite is supported by the [ble_monitor](https://github.com/custom-components/ble_monitor) Home Assistant custom component - please refer to the [docs](https://custom-components.github.io/ble_monitor/by_brand#rbaron). This custom component gets Home Assistant to automatically discover nearby b-parasites based on their advertisement data.

### Linux/Raspberry Pi & macOS
Another possibility is running [parasite-scanner](https://github.com/rbaron/parasite-scanner). It is a purpose-built bridge for b-parasites, and runs on Linux and macOS.

This is the quickest way to collect and visualize data from b-parasites, and I personally use it a lot for testing and debugging.

## Protocol and Data Encoding
Sensor data is transmitted via BLE advertisement broadcasts. [Here](./code/b-parasite/README.md) you can find a byte-by-byte description of the data as it is encoded inside the advertisement packet.

# Battery Life
**tl;dr:** By taking readings 10 minutes apart, the battery should last for over a couple of years.

The main parameters involved in estimating the battery life are:
* Current consumption (both in operation and during sleep)
* Duty cycle (how much time it spends in operation vs. sleeping)
* Battery capacity - this is roughly 200 mAh for CR2032 cells

Details and power comsumption measurements in the #48. [This spreadsheet](https://docs.google.com/spreadsheets/d/157JQiX20bGkTrlbvWbWRrs_WViL3MgVZffSCWRR7uAI/edit#gid=0) can be used to estimate battery life for different parameters.

<p align="center">
  <img src="img/resized/img2.jpg" border="0" alt="b-parasite stuck into a small plant vase" />
</p>

# Case
A 3D printable case model can be found in [case/](case/).
![Render of the 3D printable case](./img/case/screenshot.png)

# License
The hardware and associated design files are released under the [Creative Commons CC BY-SA 4.0 license](https://creativecommons.org/licenses/by-sa/4.0/).
The code is released under the [MIT license](https://opensource.org/licenses/MIT).
