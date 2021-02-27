MQTT
====

To setup mqtt broadcasting the controller needs to be
connected to a wifi network in order to find the mqtt borker.
Additionally you need to configure at least the host and the
port of the mqtt broker. If needed you also have to provide
the credentials.

The easiest way is to use the neobee command that is available
after you installed the library. The process of configuration is
explain in :ref:`configuring_the_board`.

Now delete all rows in the config file beside the mqtt rows and the firmware version. Now fill
in the apropiatecvalues. If you don't need credentials simply drop
those lines too.

.. code-block:: json

    {
    "firmware_version": "0.1.1",
    "mqtt_host": "broker.hivemq.com",
    "mqtt_port": 1883
    }

Now load back the configuration to the board and don't forget to save
the configuration. Othwerwise it will be lost after a reboot.

.. code-block:: bash

    neobee 192.168.4.1 -i configuration.json --save --reset


The Payload
-----------

Every mqtt message coems with a payload. The payload contains the
data. For neobee all informations are packed in one payload. This
ensures, that you can be sure, that all data belongs to the same
timestamp. Also the traffic is reduced. On the other hand, you have
split the bytes to get the detailed data.

Every time the controller publishes its information, the data
is packed into an 18 byte long payload.

Byte 0-5
    contain the 6 bytes of the controllers MAC address

Byte 6-9
    contain the weight as a :ref:`f100 <f100>` value

Byte 10-13
    contain the temperature inside as a :ref:`f100 <f100>` value

Byte 14-17
    contain the temperature outside as a :ref:`f100 <f100>` value


Topics
------

When the board connects to the mqtt server it publishes a messagewith
the topic `/neobee/hive/connect`. The payload contains the MAC address
[6 bytes], the IP [4 bytes] and the name [20 bytes] of the board.

Everytime a measurement is triggered, a message with the topic `/neobee/hive/rawdata`
is send. The payload contains the data as desribed above.
