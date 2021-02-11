# parasite
A low power soil moisture sensor based on the nRF52840.

# TODO
* Implement BLE advertising with moisture, battery level
* Figure out how to calibrate the ADC when running from different voltages, as the battery discharges
* Implement deep sleep
* Measure current in deep sleep
* Measure current in operation (ADC + BLE adversiting)
* Test with a range of 2-5V input. Disconnect from USB and monitor using a serial post. Use the bench power supply to power Vcc with variable voltage. *DO NOT CONNECT VCC FROM THE USB-TO-SERIAL BOARD!**
* Test with a coin cell
* Implement ADC for battery monitoring
* Figure out a way for people to configure the device with a custom name. Idea: BLE service (this is what my [hacked xiaomi temp sensor](https://github.com/atc1441/ATC_MiThermometer) does)
* Figure out how OTA works (if at all) over BLE
* Design new board using the nrf52 instead of esp32

# Done
* Implement ADC for the parasitic capacitor; check out air/water range (using protoboard)
* Simple PWM square wave generator
* Hook square wave generator to the protoboard sensor circuit
* Make the protoboard sensor work