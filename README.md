# neobee
Condition monitoring and data analytics for bee hives.

## Goals of the project

Condition monitoring of a bee hive. Tracking weight, temperatures and 
maybe additional sensor data and pushing the data to an mqtt broker.

The project consist of the folloeing hardware components:

* Wemos D1 - ESP8266 based controller
* HX711 24-Bit Analog-to-Digital Converter (ADC) 
* Load cell from BOSCH
* DS18B20 - OneWire digital temperature sensor from dallas

## Libraries

### HX711
Information about the HX711 library can be found here: https://github.com/bogde/HX711
Tested with Version 0.7.2

### DS18B20
Information about the DS18B20 library can be found here: https://github.com/milesburton/Arduino-Temperature-Control-Library
Tested with Version 3.8.0

## Roadmap

### Version 1.0

MVP version. This will be the first working version to be used in the field. THis version will have no convenience functions for setting up the wifi connection or any other additional parameters. The following features will be available:

* Calibrating the load cell (No convenience. More an explanation on how to do it.)
* Reading the weight
* Reading the inside temperature
* Reading the outside temperature
* Publish the data to an mqtt broker.
* Using deep sleep to reduce power consumtion.

