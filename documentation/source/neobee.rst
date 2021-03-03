NeoBee Command
==============

After installing the NeoBee python library, you can use the neobee
tool to manage the controller. Under linux you can call the neobee
command directly. On windows you need to use it like this:

.. code:: python

    python -m neobee.neobee

The neobee command is the swiss army knife tool to manage your device.
In this chapter we will have look at all the different options.

Every command needs the IP of the board. In this documentation I will
use ``192.168.178.30``. Of course you have to adopt this to the correct
IP of your board. 

.. note::

    The neobee programm works only, if the controller is in command
    mode.

Display the help message
------------------------

To display the integrated help message just call:

.. code:: bash

    (venv) $ neobee --help

Print the current configuration
-------------------------------

To display the current configuration simply call:

.. code:: bash

    (venv) $ neobee --dump 192.168.178.30


Export the current configuration
--------------------------------

The following command exports the current configuration
to the specified file. If the file already exists, it
will be overwritten.

.. code:: bash

    (venv) $ neobee -o configuration.json 192.168.178.30

