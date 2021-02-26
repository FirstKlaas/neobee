class MacAddress:
    def __init__(self, data):
        self._addr = data[0:6]

    def __getitem__(self, index: int) -> int:
        return self._addr[index]

    def __setitem__(self, index: int, val: int):
        self._addr[index] = val

    def __str__(self):
        return ":".join("{:02x}".format(x) for x in self._addr)

    def __repr__(self):
        return "MacAddress(" + ":".join("{:02x}".format(x) for x in self._addr) + ")"


class IPAddress:
    def __init__(self, data):
        self._addr = data[0:4]

    def __getitem__(self, index: int) -> int:
        return self._addr[index]

    def __setitem__(self, index: int, val: int):
        self._addr[index] = val

    def __str__(self):
        return ".".join(f"{x}" for x in self._addr)
