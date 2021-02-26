NeoBeeShell
===========

NeoBeeShell is the main class to interact with the controller
programmatically. 

Here's an example:

.. code:: python

    with NeoBeeShell(host="192.168.178.80") as s:
        name = s.name
        s.wifi_password = "mysecret"
        s.save_configuration()


.. autoclass:: neobee.shell.NeoBeeShell
    :members:

