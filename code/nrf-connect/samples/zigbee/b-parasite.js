// This is a zigbee2mqtt zigbee2mqtt.io converter for this sample.
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
    fromZigbee: [fz.temperature, fz.humidity, fz.battery],
    toZigbee: [],
    exposes: [e.temperature(), e.humidity(), e.battery()],
    configure: async (device, coordinatorEndpoint, logger) => {
        const endpoint = device.getEndpoint(10);
        await reporting.bind(
            endpoint,
            coordinatorEndpoint,
            ['genPowerCfg', 'msTemperatureMeasurement', 'msRelativeHumidity']);
        await reporting.batteryPercentageRemaining(endpoint);
        await reporting.temperature(endpoint);
        await reporting.humidity(endpoint);
    }
};

module.exports = definition;