from ipaddress import IPv4Address

import paho.mqtt.client as mqtt

from neobee.util import uint32_to_float
from neobee.net import MacAddress, IPAddress

def handle_connect(data):
    mac_addr = MacAddress(data[0:6])
    ip_addr = IPAddress(data[6:10])

    offset = uint32_to_float(data[10:14])
    factor = uint32_to_float(data[14:18])
    
    print()
    print("NeoBee device connected")
    print(f" => MAC            = {mac_addr}")
    print(f" => IP             = {ip_addr}")
    print(f" => Offset         = {offset}")
    print(f" => Factor         = {factor}")

def handle_rawdata(data):
    mac_addr = MacAddress(data[0:6])
    weight = uint32_to_float(data[6:10])
    temp_inside = uint32_to_float(data[10:14])
    temp_outside = uint32_to_float(data[14:18])

    print()
    print("Received sensor data")
    print(f" => MAC            = {mac_addr}")
    print(f" => Weight         = {weight}")
    print(f" => Temp (inside)  = {temp_inside}")
    print(f" => Temp (outside) = {temp_outside}")

commands = {
    "connect" : handle_connect,
    "rawdata" : handle_rawdata,
}

def on_message(client: mqtt.Client, userdata, message):
    command = message.topic.split("/")[-1:][0]
    f = commands.get(command, None)
    if f is not None:
        f(message.payload)

client = mqtt.Client("dumper")
client.on_message = on_message
client.connect("192.168.178.77")
client.subscribe("/neobee/hive/#")

client.loop_forever()