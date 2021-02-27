import json
from datetime import datetime
from shutil import copyfile

data = {}

with open("info.json", "r") as f:
    data = json.load(f)

version = data["version"]

def copy_firmware():
    version_string = ".".join([format(x) for x in version])
    copyfile(".pio/build/wemos_debug/firmware.bin", "firmware/neobee_wemos_latest-debug.bin")
    copyfile(".pio/build/wemos_debug/firmware.bin", f"firmware/neobee_wemos_{version_string}-debug.bin")
    copyfile(".pio/build/wemos_release/firmware.bin", "firmware/neobee_wemos_latest.bin")
    copyfile(".pio/build/wemos_release/firmware.bin", f"firmware/neobee_wemos_{version_string}.bin")

if __name__ == "__main__":
    copy_firmware()
