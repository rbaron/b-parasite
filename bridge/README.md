# What's a Bridge (or a Hub)?
b-parasite periodically broadcasts its sensors readings via bluetooth low energy (BLE). Usually, a bridge (a separate device) is used for capturing those broadcasts, decoding them, and forwarding them somewhere else (like MQTT or directly to a database).

# An ESPHome-based Example Bridge
This directory contains a hub implementation based on the [ESPHome project](https://github.com/esphome/esphome/), which runs on the popular [ESP32](https://www.espressif.com/en/products/socs/esp32) microcontrollers.

The `parahub.yaml` file defines an ESPHome project using the `b_parasite` platform. This platform has not yet been merged in the official ESPHome repository, so if you want to use this example bridge, you'll need to use the [rbaron/esphome](https://github.com/rbaron/esphome) fork.

## Secrets
`parahub.yaml` contains some directives such as `!secret mqtt_password`, which instructs it to read secrets from a `secrets.yaml` file.

This file is usually not commited to version control, so you'll need to populate your own. Here's an example:

```yaml
# Example secrets.yaml
wifi_ssid: WIFI_NETWORK_NAME
wifi_password: WIFI_PASSWORD
mqtt_broker: MQTT_BROKER
mqtt_password: MQTT_PASSWORD
ota_password: OTA_PASSWORD
```

# Alternative
[parasite-scanner](https://github.com/rbaron/parasite-scanner) is a purpose-built BLE-MQTT bridge for b-parasites, and runs on Linux and macOS.