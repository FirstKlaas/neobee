.. _getting_started:

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

.. youtube:: lE4uKr7uM5Y

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

    esptool.py --port /dev/ttyUSB0 --baud 460800 write_flash --flash_size=detect 0 ../neobee_latest.bin
    
.. code-block:: bash

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

Of course you have to adopt the device to whatever is the right one for you.

If everything worked fine. The LED of yout NeoBee Board should be on. Also a new
wifi network shows up ``NeoBee``. Congratulations!!! Your board is working.
Maybe a good moment for a cup of tea.

Checking the log messags with a teminal
---------------------------------------

If you flashed the debug version of the fimware, you can watch the log
messages with a terminal app.

Here I use `screen` to connect to the board. Of course this works only,
if the board is connected to your computer via an usb cable.

In a linux terminal type in:

.. code:: bash

    screen /dev/ttyUSB0 9600

The 9600 is important, because the speed is part of the firmware and
cannot be changed (unless you chang the source code).

Here's a short video on how this will look like.After starting `screen`
you need to press the `reset button` of your board.

.. youtube:: ynODvW_O63o

.. _configuring_the_board:

Configuring the board
---------------------

Next step is to configure the board. If you have started with a fresh installation of the firmware,
the controller is not configured.
For example the controller does not know which wifi networt to connect to.

Therefore the controller automatically boots into AP (Access Point) mode. You should find a new 
WiFi network called "NeoBee" you can connect to without the need of any credentials.

At least for the configuration of the wifi network, the computer needs to be connected to this
neobee hotspot. 

.. note::
    Of course the controller does not act as a router or dns server. So beware of the fact that
    as long as you are connected to the controller you cannot access the internet or other lokal
    ressources.
 
The easiest way is to download the current
configuration, which is of course empty, but serves as a template. When the board
acts as an AP, the IP for the board is ``192.168.4.1``.

.. note::
    Connect to the network called NeoBee!

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
probably, you will not know the correct values for the
``scale_factor`` and the ``scale_offset``. We will determine
these values in the in the
:ref:`taring and calibrating the scale <taring-and-calibrating>`
tutorial.

After saving out your changes, you can know easily configure
your board using the configuration file.

.. note::
    Delete the line you don't want to change or have no value bevor writing them back.

A configuration file to just set the name and the wifi credentials woul look like:

.. code:: json

    {
        "firmware_version": "0.0.1", 
        "device_name": "Neo1",
        "ssid": "myssid",
        "password": "mypassword"
    }


To then configure the board just type the following command:

.. code-block:: bash

    neobee 192.168.4.1 -i configuration.json --save --reset

The board will reset and connect to the configured wifi network.
If the board is not able to connect to the wifi network, it will
again setup an access point.

.. _github_firmware: https://github.com/FirstKlaas/neobee/tree/master/neobee_firmware/firmware
