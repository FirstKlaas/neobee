.. _taring-and-calibrating:

Taring and Calibrating the Scale
================================

The load cell is connected via a HX711 analog-to-digital
converter. It converts the weigth to an 24 bit value.
Bevor we can use it as an scale, we have to calibrate
the sensor values.

First we need the offste. The offset is the value, we
measure, wenn the scale is *empty*. This is also known as
taring. Make shure, you know how to
:ref:`enter the command mode<entering-command-mode>`.

Currently configuration is done via python. If you haven't
set up the python environment, please read the instructions
on :ref:`setting-up-the-python-library`.

To tare the weight, follow the steps:

1. Connect the board via USB to your computer.
2. Boot the board into :ref:`command mode <entering-command-mode>`.
3. Press and hold the ``CMB`` Button
4. *Now the python part*
