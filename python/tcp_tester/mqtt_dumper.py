from ipaddress import IPv4Address

import paho.mqtt.client as mqtt

def hello():
    print("Hello")

def _print_buffer(buffer):
    print(":".join("{:02x}".format(x) for x in buffer))

def uint32_to_float(data):
    return ((data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3]) / 100

def handle_connect(data):
    mac = data[0:6]
    ip = data[6:10]
    ipaddr = IPv4Address(ip[0] << 24 | ip[1] << 16 | ip[2] << 8 | ip[3])
    print(ipaddr)
    mac_addr = ":".join([format(x, "02X") for x in mac])
    print(mac_addr)
    offset = uint32_to_float(data[10:14])
    factor = uint32_to_float(data[14:18])

def handle_rawdata(data):
    mac = data[0:6]
    weight = uint32_to_float(data[6:10])
    temp_inside = uint32_to_float(data[10:14])
    temp_outside = uint32_to_float(data[14:18])
    print(f"Received sensor data: Weight={weight}, Temp (inside)={temp_inside}, Temp (outside) = {temp_outside}")

commands = {
    "connect" : handle_connect,
    "rawdata" : handle_rawdata,
}

def on_message(client: mqtt.Client, userdata, message):
    print("------")
    command = message.topic.split("/")[-1:][0]
    f = commands.get(command, None)
    if f is not None:
        f(message.payload)

    print("Command", command)

client = mqtt.Client("dumper")
client.on_message = on_message
client.connect("192.168.178.77")
client.subscribe("/neobee/hive/#")

client.loop_forever()