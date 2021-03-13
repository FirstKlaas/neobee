# Version 0.1.7

## Dependencies

- OnewWire@3.3.5
- DallasTemperature@3.9.1
- bogde/HX711@0.7.4
- PubSubClient@2.8

## Changes
- If no scale is present, the tare command returns BAD_REQUEST
- Offset can be negative. Until now only positive values have been stored. This is fixed. In fact offset may not be negative, but the HX711 mixes signed and usigned long values. So tare returns an signed long which can be a large unsigned long, interpretated as negative.
- For the time been the different calcuation methods for the weight command are disabled and the library function ``read_units`` is used. 

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

