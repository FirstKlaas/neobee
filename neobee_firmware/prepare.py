import json
from datetime import datetime
from shutil import copyfile

data = {}

with open("info.json", "r") as f:
    data = json.load(f)

version = data["version"]

def generate_version_header():
    with open("include/neobeeVersion.h", "w") as f:
        f.write("/**********************************\n")
        f.write(" Auto generated file. Do not edit.\n")
        f.write("\n")
        f.write(f" Generated at {datetime.now().isoformat(timespec='minutes')}\n")
        f.write("**********************************/\n")

        f.write(f"#define MAJOR_VERSION {version[0]}\n")
        f.write(f"#define MINOR_VERSION {version[1]}\n")
        f.write(f"#define BUILD_VERSION {version[2]}\n")
        
def copy_firmware():
    version_string = ".".join([format(x) for x in version])
    copyfile(".pio/build/wemos_debug/firmware.bin", "firmware/neobee_wemos_latest-debug.bin")
    copyfile(".pio/build/wemos_debug/firmware.bin", f"firmware/neobee_wemos_{version_string}-debug.bin")
    copyfile(".pio/build/wemos_release/firmware.bin", "firmware/neobee_wemos_latest.bin")
    copyfile(".pio/build/wemos_release/firmware.bin", f"firmware/neobee_wemos_{version_string}.bin")

if __name__ == "__main__":
    generate_version_header()
    copy_firmware()
