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
        
if __name__ == "__main__":
    generate_version_header()