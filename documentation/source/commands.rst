Commands
========

The NeoBee board defines its own communication protocol based on tcp/ip.
The board acts as a server. It supports all commands needed to configure
the board as well as read the sensore data.

Paket structure
---------------

All pakets have a fixed sizeof 32 bytes. The first byte of a request packet
contains the commande code. The second byte contains command flags. For
details read the related command documentation. The rest of the bytes contain
the data.

Get Name
--------

Getting the human readable name of the board. The name of the board
should be unique, although it cannot be guaranteed by the board itself.

Only ASCII_ characters are supported.

===== ================================
Byte  Description
===== ================================
00    Command byte. Always 001
01    Request Modifier. Always 0
02-29 0 [*unused*]
===== ================================

Set Name
--------

Setting the human readable name of the board. THe maximum langth of
th name is 30 bytes. All unused bytes in the request paket must be
set to 0.

===== ================================
Byte  Description
===== ================================
00    Command byte. Always 002
01    Status byte.
02-29 Name (Unused bytes are zeroed)
===== ================================

Get Flags
---------

Getting the flags., indicating the state of certain settings.

===== ================================
Byte  Description
===== ================================
00    Command byte. Always 003
01    Request Modifier. Always 0
02-29 0 [*unused*]
===== ================================

Reset Settings
--------------

Clears all settings. After rebooting the system, the board will 
automatically go into command mode.

===== ================================
Byte  Description
===== ================================
00    Command byte. Always 004
01    Request Modifier. Always 0
02-29 0 [*unused*]
===== ================================

Save Settings
-------------

Saves the settings to the EEPROM.

===== ================================
Byte  Description
===== ================================
00    Command byte. Always 5
01    Request Modifier. Always 0
02-29 0 [*unused*]
===== ================================

Erase Settings
--------------

Similar to reset settings, baut also removes the magic bytes from
EEPROM. 

===== ================================
Byte  Description
===== ================================
00    Command byte. Always 6
01    Request Modifier. Always 0
02-29 0 [*unused*]
===== ================================


Reset Board
-----------

Rests the board. To be more precise, restarts the board. After a reset,
all changes made to the board will take effect.

===== ================================
Byte  Description
===== ================================
00    Command byte. Always 7
01    Request Modifier. Always 0
02-29 0 [*unused*]
===== ================================

Get Scale Offset
----------------

Returns the current offset of the scale. The offset defines the 0 level
of the weight. Withe the `tare` command, you can 

===== ================================
Byte  Description
===== ================================
00    Command byte. Always 10
01    Request Modifier. Always 0
02-29 0 [*unused*]
===== ================================


Set Scale Offset
----------------

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

Get Scale Factor
----------------

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

Set Scale Factor
----------------

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

.. _ASCII: https://www.ascii-code.com/

