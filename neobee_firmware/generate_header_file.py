import json
from datetime import datetime
from shutil import copyfile


def generate_version_header():
    data = {}

    with open("info.json", "r") as f:
        data = json.load(f)

    version = data["version"]
    version[2] = version[2] + 1
    data["version"] = version

    print(f"Updated to version {version}")

    with open("info.json", "w") as f:
        json.dump(data,f, indent=2)

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
