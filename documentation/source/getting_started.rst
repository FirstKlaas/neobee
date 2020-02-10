Getting started
===============

First of course you need to get the hardware part prepared.
If not, first go to the hardware section.

Currently the documenation only takes linux into accounte.
But if you are familiar with windows, most things should
work quite similar.

Flashing the Firmware
---------------------

First you need to flash the neobee firmware to your wemos.
Every version of the firmware comes as a debug and a production
version. You can download the firmware from `github <github_firmware>`_.

Or, if you prefer to use curl:

.. code-block:: bash

    curl -L https://github.com/FirstKlaas/neobee/raw/master/neobee_firmware/firmware/neobee_wemos_latest-debug.bin --output neobee_latest.bin


Although not really necessary, I emphasize to do the installation of
the library in a virtual environment. If you do no not how to setup a
virtual environment, do q quick search on the internet. There are
dozens of HOWTOs.

In a nutshell is is something like this:

.. code-block:: bash

    mkdir neobee
    cd neobee
    python3 -m venv venv
    source ./venv/bin/activate

Now you can use pip to install esptool and the neobee library and tools.
I normally install the wheel library first.

.. code-block:: bash

    pip install wheel
    pip install esptool
    pip install neobee

That's everything you need.

Now you can flash the firmware you just downloaded:

.. code-block:: bash

    esptool.py --port /dev/ttyUSB0 --baud 460800 write_flash --flash_size=detect 0 noebee_latest.bin
    esptool.py v2.8
    Serial port /dev/ttyUSB0
    Connecting....
    Detecting chip type... ESP8266
    Chip is ESP8266EX
    Features: WiFi
    Crystal is 26MHz
    MAC: 84:f3:eb:90:6d:c2
    Uploading stub...
    Running stub...
    Stub running...
    Changing baud rate to 460800
    Changed.
    Configuring flash size...
    Auto-detected Flash size: 4MB
    Compressed 294880 bytes to 211650...
    Wrote 294880 bytes (211650 compressed) at 0x00000000 in 4.8 seconds (effective 495.9 kbit/s)...
    Hash of data verified.

    Leaving...
    Hard resetting via RTS pin...

Of course you have to adopt the port to whatever is the right one for you.

If everything worked fine. The LED of yout NeoBee Board shoul be on. Also a new
wifi network shows up ``NeoBee``. Congratulations!!! Your board is working.
Maybe a good moment for a cup of tea.

Configure the board
-------------------

Next step is to configure the board. The easiest way is to download the current
configuration, which is of course empty, but serves as a template. When the board
acts as an AP, the IP for the board is ``192.168.4.1``.

To download the configuration, type the following command:

.. code-block:: bash

    neobee 192.168.4.1 -o configuration.json

The generated file should look like this (the firmware version may differ):


.. code-block:: json

    {
    "firmware_version": "0.1.1",
    "device_name": null,
    "mac_address": "84:f3:eb:90:6d:c2",
    "ssid": null,
    "password": null,
    "deep_sleep_seconds": 30,
    "scale_offset": null,
    "scale_factor": null,
    "mqtt_host": null,
    "mqtt_port": null,
    "mqtt_login": null,
    "mqtt_password": null
    }

Leave the ``firmware_version`` and the ``mac_adress`` entry
as they are and adopt all other setting to your needs. Most
probably, you will not knoe the correct values for the
``scale_factor`` and the ``scale_offset``. We will determine
these values in the in the
:ref:`taring and calibrating the scale <taring-and-calibrating>`
tutorial.

After saving out your changes, you can know easily configure
your board using the configuration file.

Just type the following command:

.. code-block:: bash

    neobee 192.168.4.1 -i configuration.json --save --reset

The board will reset and connect to the configured wifi network.
If the board is not abled to connect to the wifi network, it will
again setup an access point.

.. _github_firmware: https://github.com/FirstKlaas/neobee/tree/master/neobee_firmware/firmware