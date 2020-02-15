#!/usr/bin/env python

from neobee.neobee import command_line
from neobee.shell import NeoBeeShell, NeoBeeInfo

"""
with NeoBeeShell(host='192.168.178.72') as shell:
    info = shell.info
    print("Name set                : ", info.name_set)
    print("Wifi SSID set           : ", info.wifi_ssid_set)
    print("Wifi password set       : ", info.wifi_password_set)

    print("Number of temp. sensors : ", info.number_of_temperature_sensors)
    
"""


command_line()