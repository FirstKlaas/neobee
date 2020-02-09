import argparse
import json

from .shell import NeoBeeShell
from .error import NeoBeeError


def command_line():
    parser = argparse.ArgumentParser()
    parser.add_argument("host", help="The NeoBee host, to connect to.", type=str)
    parser.add_argument("-v", "--verbose", help="Show some more output.", action="store_true")
    parser.add_argument("--reset", help="Reset the board", action="store_true")
    parser.add_argument("--erase", help="Erase the configuration data", action="store_true")
    parser.add_argument("-s", "--save", help="Save configuration data", action="store_true")

    parser.add_argument("-n", "--name", help="The name of the neobee board", type=str)

    parser.add_argument("--ssid", help="THe wifi network to connec to", type=str)
    parser.add_argument("--password", help="The wifi password", type=str)

    parser.add_argument("--mqtt-host", help="The mqtt host", type=str)
    parser.add_argument("--mqtt-port", help="The mqtt port", type=int)
    parser.add_argument("--mqtt-login", help="The mqtt login", type=str)
    parser.add_argument("--mqtt-password", help="The mqtt password", type=str)

    parser.add_argument("--scale-offset", help="The scale offset", type=int)
    parser.add_argument("--scale-factor", help="The scale factor", type=int)

    parser.add_argument("-o", "--out-file", help="Writing the settings to", type=str)
    parser.add_argument("-i", "--in-file", help="Reading the settings from", type=str)

    args = parser.parse_args()
    host = "192.168.4.1"

    if args.host != "default":
        host = args.host

    if args.verbose:
        print(f"Connecting to host {host}")

    try:
        with NeoBeeShell(host=host) as shell:

            if args.in_file:
                with open(args.in_file, "r") as f:
                    data = json.load(f)
                    if "device_name" in data:
                        shell.name = data["device_name"]
                    if "ssid" in data:
                        shell.ssid = data["ssid"]
                    if "password" in data:
                        shell.wifi_password = data["password"]
                    if "deep_sleep_seconds" in data:
                        shell.deep_sleep_seconds = data["deep_sleep_seconds"]
                    if "scale_offset" in data:
                        shell.scale_offset = data["scale_offset"]
                    if "scale_factor" in data:
                        shell.scale_factor = data["scale_factor"]
                    if "mqtt_host" in data:
                        shell.mqtt_host = data["mqtt_host"]
                    if "mqtt_password" in data:
                        shell.mqtt_password = data["mqtt_password"]
                    if "mqtt_login" in data:
                        shell.mqtt_login = data["mqtt_login"]
                    if "mqtt_password" in data:
                        shell.mqtt_password = data["mqtt_password"]
                    if "mqtt_port" in data:
                        shell.mqtt_port = data["mqtt_port"]

            if args.name:
                if args.verbose:
                    print(f"Changing name from {shell.name} to {args.name}")

                shell.name = args.name

            if args.erase:
                if args.verbose:
                    print("Erase settings.")
                shell.erase_settings()

            if args.ssid:
                if args.verbose:
                    print(f"Setting ssid to {args.ssid}.")
                shell.ssid = args.ssid

            if args.password:
                if args.verbose:
                    print(f"Setting wifi password to {args.password}.")
                shell.wifi_password = args.password

            if args.mqtt_host:
                shell.mqtt_host = args.mqtt_host

            if args.mqtt_port:
                shell.mqtt_port = args.mqtt_port

            if args.mqtt_login:
                shell.mqtt_login = args.mqtt_login

            if args.mqtt_password:
                shell.mqtt_password = args.mqtt_password

            if args.scale_factor:
                shell.scale_factor = args.scale_factor

            if args.scale_offset:
                shell.scale_offset = args.scale_offset

            if args.save:
                if args.verbose:
                    print("Save settings.")

                shell.save_settings()

            if args.out_file:
                with open(args.out_file, "w") as f:
                    json.dump(shell.to_dict(), f, indent=2)

            if args.reset:
                if args.verbose:
                    print("Resetting board.")
                shell.reset()

    except NeoBeeError as e:
        print(e)


"""
    with NeoBeeShell(host="192.168.178.72") as shell:
        #with NeoBeeShell() as shell:

        shell.name = "NeoBee.One"
        shell.ssid = "RepeaterOben24"
        shell.set_password("4249789363748310")
        shell.deep_sleep_seconds = 30
        shell.set_scale_offset(61191.0)
        shell.set_scale_factor(21.88)
        shell.activate_wifi_sta()
        
        #shell.mqtt_password = None
        #shell.mqtt_login = None
        #shell.mqtt_host = "192.168.178.77"
        #shell.mqtt_port = 1883
        
        #shell.mqtt_password = "encowa"
        #shell.mqtt_login = "encoway"
        #shell.mqtt_host = "vm-web.lenze.digital"
        #shell.mqtt_port = 1883
        
        
        #with open("board.neobee", "w") as f:
        #    json.dump(shell.to_dict(), f, indent=2)
        
        #shell.save_settings()
        #print(json.dumps(shell.to_dict(), indent=2))
"""
