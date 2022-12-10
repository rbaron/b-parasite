// This is a zigbee2mqtt zigbee2mqtt.io converter for this sample.
// See https://www.zigbee2mqtt.io/advanced/support-new-devices/01_support_new_devices.html
// on how to use it.

const fz = require('zigbee-herdsman-converters/converters/fromZigbee');
const tz = require('zigbee-herdsman-converters/converters/toZigbee');
const exposes = require('zigbee-herdsman-converters/lib/exposes');
const reporting = require('zigbee-herdsman-converters/lib/reporting');
const extend = require('zigbee-herdsman-converters/lib/extend');
const e = exposes.presets;
const ea = exposes.access;

const definition = {
    zigbeeModel: ['b-parasite'],
    model: 'b-parasite',
    vendor: 'b-parasite',
    description: 'IoT development board - Zigbee sample',
    fromZigbee: [fz.temperature, fz.humidity, fz.battery, fz.soil_moisture, fz.illuminance],
    toZigbee: [],
    exposes: [
        e.temperature(),
        e.humidity(),
        e.battery(),
        e.soil_moisture(),
        e.illuminance_lux()],
    configure: async (device, coordinatorEndpoint, logger) => {
        const endpoint = device.getEndpoint(10);
        await reporting.bind(
            endpoint,
            coordinatorEndpoint, [
                'genPowerCfg',
                'msTemperatureMeasurement',
                'msRelativeHumidity',
                'msSoilMoisture',
                'msIlluminanceMeasurement',
            ]);
        await reporting.batteryPercentageRemaining(endpoint);
        // Not reportable :(
        // await reporting.batteryVoltage(endpoint);
        await reporting.temperature(endpoint);
        await reporting.humidity(endpoint);
        await reporting.soil_moisture(endpoint);
        await reporting.illuminance(endpoint);
    }
};

module.exports = definition;