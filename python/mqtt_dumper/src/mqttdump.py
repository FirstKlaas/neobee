"""
Simple dumper of the neobee board messages
to a human readable form.

Copyright FirstKlaas 2021
"""
import paho.mqtt.client as mqtt


def on_message(client, userdata, message):
    #print("message received ")
    #print("message topic=",message.topic)
    #print("message qos=",message.qos)
    #print("message retain flag=",message.retain)
    def shift_add(a, b):
        return a * 8 + b
    
    if message.topic == "/neobee/hive/disconnect":
        print("Device disconnected: ", message.payload)
    elif message.topic == "/neobee/hive/connect":
        print("Device connected:")
    elif message.topic == "/neobee/hive/rawdata":
        print("New data")
        mac = ":".join([hex(b)[2:4].upper() for b in message.payload[:6]])
        print("Mac Address: ", mac)
        from functools import reduce
        weight = reduce(lambda a,b: a*8+b, message.payload[6:10]) / 100
        print("Weight: ", weight)

        
    else:
        print("Unhandled topic ", message.topic)

if __name__ == "__main__":
    client = mqtt.Client("smd")
    client.connect("broker.hivemq.com")
    client.on_message = on_message
    client.subscribe("/neobee/#")
    client.loop_forever()