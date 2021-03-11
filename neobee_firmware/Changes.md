# Version 0.1.7

## Dependencies

- OnewWire
- DallasTemperature@3.9.1
- bogde/HX711@0.7.4
- PubSubClient@2.8

## Changes

# Version 0.1.6

## Dependencies

- OnewWire
- DallasTemperature@3.9.1
- bogde/HX711@0.7.4
- PubSubClient@2.8

## Changes

- The mqtt reconnect never was called in case the connection got lost. Now the connection is checked every time, before data is published.
- Dependencies upgraded.
- Some smaller bugfixes.
- Temperature values are read based on the address and not based on the index, which is a bit faster.
- If a temperature value reads 85.0, 0 is returned, as 85.0 indicates an error.#

