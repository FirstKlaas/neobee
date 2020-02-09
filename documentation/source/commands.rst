Commands
========

The NeoBee board defines its own tcp based communication protocol
based on tcp/ip. The board acts as a server. It supports all
commands needed to configure the board as well as read the sensore
data.

Paket structure
---------------

All pakets have a fixed size of 32 bytes. The first byte of a request packet
contains the commande code. The second byte defines the method. For
details read the related command documentation. The rest of the bytes contain
the data.

Request Method
--------------

The two least bits of the second byte define the request method. Following
methods are possible.

=== ======= ===============================================================
0   NONE    Method not used.
1   GET     The method to read values
2   PUT     Method used to write/change values
3   DELETE  Method used to delete/reset values
=== ======= ===============================================================


[01] Get Name
-------------

Getting the human readable name of the board. The name of the board
should be unique, although it cannot be guaranteed by the board itself.

Only ASCII_ characters are supported.

===== ================================
Byte  Description
===== ================================
00    Command byte. Always 1
01    Request Modifier. Always 0
02-29 0 [*unused*]
===== ================================

[01] Name
---------

Managing the human readable name of the board. Supported methods are
``GET``, ``PUT``, ``DELETE``. The maximum length of the name is 30
bytes. Because there is no way to define the encoding, the bytes are
copied directly. An ASCII encoding is highly emphasized.

Request
~~~~~~~

===== ================================
Byte  Description
===== ================================
00    Command byte. Always 2
01    Method (GET / PUT / DELETE)
02-29 Name in case of method ``PUT``. Zeroed for the other methods.
===== ================================

Response
~~~~~~~~

===== ================================
Byte  Description
===== ================================
00    Command byte. Always 2
01    Response Status
02-29 Name in case of method ``GET``. Zeroed for the other methods.
===== ================================

[03] Get Flags
--------------

Getting the flags., indicating the state of certain settings.

===== ================================
Byte  Description
===== ================================
00    Command byte. Always 3
01    Request Modifier. Always 0
02-29 0 [*unused*]
===== ================================

[04] Reset Settings
-------------------

Clears all settings. After rebooting the system, the board will
automatically go into command mode.

===== ================================
Byte  Description
===== ================================
00    Command byte. Always 4
01    Request Modifier. Always 0
02-29 0 [*unused*]
===== ================================

[05] Save Settings
------------------

Saves the settings to the EEPROM.

===== ================================
Byte  Description
===== ================================
00    Command byte. Always 5
01    Request Modifier. Always 0
02-29 0 [*unused*]
===== ================================

[06] Erase Settings
-------------------

Similar to reset settings, baut also removes the magic bytes from
EEPROM.

===== ================================
Byte  Description
===== ================================
00    Command byte. Always 6
01    Request Modifier. Always 0
02-29 0 [*unused*]
===== ================================


[07] Reset Board
----------------

Rests the board. To be more precise, restarts the board. After a reset,
all changes made to the board will take effect.

===== ================================
Byte  Description
===== ================================
00    Command byte. Always 7
01    Request Modifier. Always 0
02-29 0 [*unused*]
===== ================================

[10] Get Scale Offset
---------------------

Returns the current offset of the scale. The offset defines the 0 level
of the weight.

===== ================================
Byte  Description
===== ================================
00    Command byte. Always 10
01    Request Modifier. Always 0
02-29 0 [*unused*]
===== ================================


[11] Set Scale Offset
---------------------

Sets the current offset of the scale. The offset defines the 0 level
of the weight.

===== ================================
Byte  Description
===== ================================
00    Command byte. Always 11
01    Request Modifier. Always 0
02-05 F100 value for the offset
06-29 0 [*unused*]
===== ================================

[12] Get Scale Factor
---------------------

Returns the current factor of the scale. The factor converts the
internal units into the external values. Check out the calibration
page for more information.

===== ================================
Byte  Description
===== ================================
00    Command byte. Always 12
01    Request Modifier. Always 0
02-29 0 [*unused*]
===== ================================

[13] Set Scale Factor
---------------------

Sets the current factor of the scale. Check out the calibration
page for more information.

===== ================================
Byte  Description
===== ================================
00    Command byte. Always 13
01    Request Modifier. Always 0
02-05 F100 value for the factor
06-29 0 [*unused*]
===== ================================

[20] Get SSID
-------------

Returns the wifi networkname name (the ssid)  to connect to. If no
ssid was configured. Returns a ``NOT_FOUND`` status.

**Request**

===== ================================
Byte  Description
===== ================================
00    Command byte. Always 20
01    Request Modifier. Always 0
02-29 0 [*unused*]
===== ================================

**Response**

===== ================================
Byte  Description
===== ================================
00    Command byte. Always 20
01    Status. ``OK`` if name was set, ``NOT FOUND`` else.
02-29 The ssid. Unused bytes are 0
===== ================================


[21] Set SSID
-------------

Set the ssid to connect to.

**Request**

===== ================================
Byte  Description
===== ================================
00    Command byte. Always 21
01    Request Modifier. Always 0
02-29 The name of the ssid. Unused bytes should be set to 0
===== ================================

[22] Clear SSID
---------------

Clears the ssid to connect to. When no ssid is set,
the board will automatically go into AP mode after reboot.
The preferred way to force AP mode is to do a
``Set Wifi Active(False)`` request.

**Request**

===== ================================
Byte  Description
===== ================================
00    Command byte. Always 22
01    Request Modifier. Always 0
02-29 0 [*unused*]
===== ================================

[23] Get Password
-----------------

Gets the currently configured wifi password.

[24] Set Password
-----------------

Sets the wifi password.

[25] Clear password *(deprecated)*
----------------------------------

Clears the wifi password. This command is deprecated and
will be removed in future releases. Please use
``set password``, where the first byte of the name is 0.

[26] Enable or disable wifi
---------------------------

Enables or disables the wifi connection.
If diabled, the board will boot into AP
mode after reboot. If enabled, it will try
to connect to the configured wifi network.
If no network is configured, the board will
also go into AP mode

[27] Get wifi flags *(deprecated)*
----------------------------------

Gets the wifi flags


.. _ASCII: https://www.ascii-code.com/

