.. _setting-up-the-python-library:

Setting up the NeoBee Python Library
====================================

Installing the NeoBee library is a piece of cake, as it is hosted on PyPi. Although
you can install globally (as root) or in user space, I recommend you to use a virtual
environment. Follow along the next steps and you're good to go.

Open a terminal and type:

.. code-block:: bash

    mkdir neobee
    cd neobee
    python3 -m venv venv
    source ./venv/bin/activate

Now you can use pip to install the neobee library.
I normally install the wheel library first.

.. code-block:: bash

    pip install --upgrade pip
    pip install --upgrade wheel
    pip install --upgrade neobee

That's everything you need.

This installs a neobee programm to configure youre device.
Try the following:

.. code-block:: bash

    neobee --help

You should get something like:

.. code-block:: bash

    usage: neobee [-h] [-v] [--reset] [--erase] [-s] [-n NAME] [--ssid SSID]
                [--password PASSWORD] [--mqtt-host MQTT_HOST]
                [--mqtt-port MQTT_PORT] [--mqtt-login MQTT_LOGIN]
                [--mqtt-password MQTT_PASSWORD] [--scale-offset SCALE_OFFSET]
                [--scale-factor SCALE_FACTOR] [-o OUT_FILE] [-i IN_FILE]
                host

    positional arguments:
    host                  The NeoBee host, to connect to.

    optional arguments:
    -h, --help                      Show this help message and exit
    -v, --verbose                   Show some more output.
    --reset                         Reset the board
    --erase                         Erase the configuration data
    -s, --save                      Save configuration data
    -n NAME, --name NAME            The name of the neobee board
    --ssid SSID                     The wifi network to connect to
    --password PASSWORD             The wifi password
    --mqtt-host MQTT_HOST           The mqtt host
    --mqtt-port MQTT_PORT           The mqtt port
    --mqtt-login MQTT_LOGIN         The mqtt login
    --mqtt-password MQTT_PASSWORD   The mqtt password
    --scale-offset SCALE_OFFSET     The scale offset
    --scale-factor SCALE_FACTOR     The scale factor
    -o OUT_FILE, --out-file OUT_FILE    Writing the settings to
    -i IN_FILE, --in-file IN_FILE   Reading the settings from

