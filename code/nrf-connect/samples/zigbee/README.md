# Zigbee firmware sample
This sample is adapted from the [zigbee_template](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/samples/zigbee/template/README.html) from the nRF Connect SDK. It's a basic experimental/educational/exploratory firmware sample for b-parasite.

## Clusters
These [clusters](https://en.wikipedia.org/wiki/Zigbee#Cluster_library) are defined in the sample:

|Cluster ID|Name|
|--------|---|
|0x0001|Power Configuration|
|0x0400|Illuminance Measurement|
|0x0402|Temperature Measurement|
|0x0405|Relative Humidity Measurement|
|0x0408|Soil Moisture Measurement|

## Pairing Mode
The sample will first boot and start looking for a Zigbee coordinator - in pairing mode. The onboard LED will be flashing once a second while in this mode. Once a suitable network is found, the LED will briefly flash 3 times and remain off.

### Factory Reset
A factory reset will make b-parasite forget its network pairing information and switch to pairing mode. There are two (mutually exclusive) methods to perform a factory reset, controlled by the `CONFIG_PRST_ZB_FACTORY_RESET_METHOD` config flag.

#### Factory Reset Method 1 (default) - Double reset
Resetting b-parasite twice in the timestamp of 5 seconds will perform a factory reset. With this method, both shorting the `RST` pin to ground and removing-inserting the battery counts as a reset.

For better results, wait > 1 and < 5 seconds second between the resets. The LED will flash a total of 8 times to indicate it worked.

#### Factory Reset Method 2 - Reset Pin
In this method, there's a distinction between two reset modes.

#### Power up mode
The device enters this mode when it is powered. For example, swapping an old battery or connecting to eternal power. This is the "usual" reset mode, and joined networks will be remembered.

#### Reset pin mode
If the device's RESET pin is briefly grounded, the device will effectively be **factory reset**. The device will leave its previous network and start looking for a new one.

While it works, this method can be finicky - an accidental pin reset will perform an unwanted factory reset.

## Configs
Available options in `Kconfig`. Notable options:
* `CONFIG_PRST_ZB_SLEEP_DURATION_SEC`: amount of time (in seconds) the device sleeps between reading all sensors and updating its clusters
* `CONFIG_PRST_ZB_PARENT_POLL_INTERVAL_SEC`: amount of time (in seconds) the device waits between polling its parent for data

## Home Assistant Integration
This firmware sample has only been tested with Home Assistant, using one of the following integrations.

### Zigbee Home Automation (ZHA)
With the [ZHA](https://www.home-assistant.io/integrations/zha) Home Assistant integration, b-parasite should work out of the box.

### Zigbee2MQTT & Home Assistant
With [Zigbee2MQTT](https://zigbee2mqtt.io/), a custom converter is required. The [b-parasite.js](b-parasite.js) file contains such a converter. See [Support new devices](https://www.zigbee2mqtt.io/advanced/support-new-devices/01_support_new_devices.html) for instructions.

## Battery Life
While sleeping, the device consumes around 2 uA:
![sleeping current](./media/power-profile/sleeping.png)
In the active cycle, it averages around 125 uA for 1 second:
![active current](media/power-profile/active.png)
