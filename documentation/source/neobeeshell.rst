.. _CONTEXTMANAGER: https://docs.python.org/3/library/stdtypes.html#typecontextmanager

NeoBeeShell
===========

NeoBeeShell is the main class to interact with the controller
programmatically. The purpose if the library is to read and write
the configuration settings. The library uses a tcp communication.
So to use the library, you do not need the serial line of the usb
cable. The benfit of this approach is, that you are able to accessthe
board remotely as long as you are in the same network.

Because a tcp socket is used, it is important to close the socket
at the end. The NeoShell class implements pythons
`context protocol <CONTEXTMANAGER>`_.
Therefore you can use it in a `with` clause, which garantuees to close the
socket. No need to open and close the socket. Thats all managed for
you in the background. It makes code safer and more human readable.
 

Here's an example:

.. code:: python

    with NeoBeeShell(host="192.168.178.80") as s:
        name = s.name
        s.wifi_password = "mysecret"
        s.save_configuration()


API Documentation
-----------------

.. autoclass:: neobee.shell.NeoBeeShell
    :members:

