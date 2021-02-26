MQTT
====

The Payload
-----------

Every time the controller publishes its information, the data
is packed into an 18 byte long payload.

- Byte 0-5 contain the 6 bytes of the controllers MAC address
- Byte 6-9 contain the weight as a f100 value
- Byte 10-13 contain the temperature inside as a f100 value
- Byte 14-17 contain the temperature outside as a f100 value
