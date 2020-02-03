def uint32_to_float(data):
    return ((data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3]) / 100


def HighByte(val: int):
    return (val >> 8) & 0xFF


def LowByte(val: int):
    return val & 0xFF


def HighWord(val: int):
    return (val >> 16) & 0xFFFF


def LowWord(val: int):
    return val & 0xFFFF


def print_hex_buffer(buffer):
    print(":".join("{:02x}".format(x) for x in buffer))
