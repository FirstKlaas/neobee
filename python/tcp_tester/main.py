from enum import IntEnum, IntFlag
import socket
import json

from binascii import hexlify


class CmdCode(IntEnum):
    NOP = 0
    GET_NAME = 1
    SET_NAME = 2
    GET_FLAGS = 3
    RESET_SETTINGS = 4
    SAVE_SETTINGS = 5
    ERASE_SETTINGS = 6

    GET_SCALE_OFFSET = 10
    SET_SCALE_OFFSET = 11
    GET_SCALE_FACTOR = 12
    SET_SCALE_FACTOR = 13

    GET_SSID = 20
    SET_SSID = 21
    CLEAR_SSID = 22
    GET_PASSWORD = 23
    SET_PASSWORD = 24
    CLEAR_PASSWORD = 25
    SET_WIFI_ACTIVE = 26
    GET_WIFI_FLAGS = 27

    GET_MQTT_HOST = 30
    SET_MQTT_HOST = 31
    GET_MQTT_PORT = 32
    SET_MQTT_PORT = 33
    SET_MQTT_ACTIVE = 36
    GET_MQTT_FLAGS = 37

    GET_MAC_ADDRESS = 80
    GET_VERSION = 81
    SET_IDLE_TIME = 82
    GET_IDLE_TIME = 83
    SET_DEEP_SLEEP = 84

    TARE = 200
    CALIBRATE = 201
    GET_WEIGHT = 202


class WiFiFlag(IntFlag):
    FLAG_SSID_SET = 1
    FLAG_PASSWORD_SET = 2
    FLAG_ACTIVE = 4
    ALL = 7


class StatusFlag(IntFlag):

    FLAG_OFFSET_SET = 1
    FLAG_FACTOR_SET = 2
    FLAG_GAIN_SET = 4
    FLAG_NAME_SET = 8
    FLAG_ADDR_INSIDE_SET = 16
    FLAG_ADDR_OUTSIDE_SET = 32
    DEEP_SLEEP_SET = 64
    WIFI_NETWORK_SET = 128


class RequestFlag(IntFlag):
    """
    Every request sends flags in the second
    byte of the request.

    These are the possible flags, which can
    be mixed to fit to your need.
    """
    AUTOSAVE_CTX = 1


class StatusCode(IntEnum):
    NONE = 0,
    OK = 1,
    BAD_REQUEST = 2,
    NOT_FOUND = 3,
    ILLEGAL_STATE = 4


class Response:

    def __init__(self, cmd: CmdCode, status: StatusCode, data):
        self.cmd: CmdCode = cmd
        self.status: StatusCode = status
        self.data = data

        print("Command: ", self.cmd)
        print("Status: ", self.status)
        print("Data: ", ':'.join("{:02x}".format(x) for x in self.data))


class Request:
    def __init__(self, cmd: CmdCode):
        self._buffer = bytearray(MSGLEN)

    @property
    def cmd(self) -> CmdCode:
        return self._buffer[0]

    @cmd.setter
    def cmd(self, cmd: CmdCode):
        self._buffer[0] = cmd


def myreceive(sckt, handler=None, max_tries=10, delay=10):
    bytes_recd = 0
    buffer = bytearray(MSGLEN)
    number_of_tries = 0

    while bytes_recd < MSGLEN:
        chunk = sckt.recv(min(MSGLEN - bytes_recd, 32))
        if chunk == b'':
            raise RuntimeError("socket connection broken")
        chunksize = len(chunk)
        buffer[bytes_recd:bytes_recd+chunksize] = chunk
        bytes_recd = bytes_recd + len(chunk)

    response = Response(buffer[0], buffer[1], buffer[2:])
    if handler is not None:
        handler(response)


class MacAddress:

    def __init__(self):
        self._addr = bytearray(6)

    def __getitem__(self, index: int) -> int:
        return self._addr[index]

    def __setitem__(self, index: int, val: int):
        self._addr[index] = val

    def __str__(self):
        return ':'.join("{:02x}".format(x) for x in self._addr)


class NotConnectedError(Exception):
    pass


class AlreadyConnectedError(Exception):
    pass


class BadRequestError(Exception):
    pass


class NeoBeeShell:

    def __init__(self, host="192.168.4.1", port=8888):
        self._socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.host = host
        self.port = port
        self._buffer = bytearray(32)
        self._connected = False

    def __enter__(self):
        self._socket.connect((self.host, self.port))
        self._connected = True
        return self

    def __exit__(self, type, value, traceback):
        self._socket.shutdown(1)
        self._socket.close()
        self._connected = False

    def connected(self):
        return self._connected

    def connect(self):
        if self.connected:
            raise AlreadyConnectedError()

        self.__enter__()

    def disconnect(self):
        if not self.connected:
            raise NotConnectedError()

        self.__exit__()

    def _clearbuffer(self):
        self._buffer[:] = [0] * 32

    def _buffer_to_string(self):
        return bytearray(filter(lambda x: x >= 32 and x <= 127, self._buffer[2:])).decode("ascii")

    def _string_to_buffer(self, val: str):
        if not val:
            return

        for index, char in enumerate(val):
            self[index] = ord(char)

    def _receive(self):
        if not self.connected:
            raise NotConnectedError()

        bytes_recd = 0
        while bytes_recd < 32:
            chunk = self._socket.recv(min(32 - bytes_recd, 32))
            if chunk == b'':
                raise RuntimeError("socket connection broken")
            chunksize = len(chunk)
            self._buffer[bytes_recd:bytes_recd+chunksize] = chunk
            bytes_recd = bytes_recd + len(chunk)

    def _send(self):
        if not self.connected:
            raise NotConnectedError()

        self._socket.send(self._buffer)
        self._receive()

    def _print_buffer(self):
        print(':'.join("{:02x}".format(x) for x in self._buffer))

    def __getitem__(self, index):
        return self._buffer[index+2]

    def __setitem__(self, index, value):
        self._buffer[index+2] = (value & 0xff)

    def get_version(self):
        if not self.connected:
            raise NotConnectedError()

        self._clearbuffer()
        self._cmd = CmdCode.GET_VERSION
        self._send()
        return (self[0], self[1], self[2])

    @property
    def _cmd(self):
        return self._buffer[0]

    @_cmd.setter
    def _cmd(self, value: CmdCode):
        self._buffer[0] = value

    @property
    def _status(self):
        return self._buffer[1]

    def get_scale_offset(self):
        if not self.connected:
            raise NotConnectedError()

        self._clearbuffer()
        self._cmd = CmdCode.GET_SCALE_OFFSET
        self._send()

        if self._status == StatusCode.OK:
            return ((self[0] << 24) | (self[1] << 16) | (self[2] << 8) | self[3]) / 100
        elif self._status == StatusCode.NOT_FOUND:
            return None
        else:
            raise RuntimeError()

    def set_scale_offset(self, value: float):
        if not self.connected:
            raise NotConnectedError()

        self._clearbuffer()
        self._cmd = CmdCode.SET_SCALE_OFFSET
        iValue = int(value*100)
        self[0] = (iValue >> 24) & 0xff
        self[1] = (iValue >> 16) & 0xff
        self[2] = (iValue >> 8) & 0xff
        self[3] = (iValue) & 0xff
        self._send()

    def get_scale_factor(self):
        if not self.connected:
            raise NotConnectedError()

        self._clearbuffer()
        self._cmd = CmdCode.GET_SCALE_FACTOR
        self._send()
        if self._status == StatusCode.OK:
            return ((self[0] << 24) | (self[1] << 16) | (self[2] << 8) | self[3]) / 100
        elif self._status == StatusCode.NOT_FOUND:
            return None
        else:
            raise RuntimeError()

    def set_scale_factor(self, value: float):
        if not self.connected:
            raise NotConnectedError()

        self._clearbuffer()
        self._cmd = CmdCode.SET_SCALE_FACTOR
        iValue = int(value*100)
        self[0] = (iValue >> 24) & 0xff
        self[1] = (iValue >> 16) & 0xff
        self[2] = (iValue >> 8) & 0xff
        self[3] = (iValue) & 0xff
        self._send()

    def get_mac_address(self):
        if not self.connected:
            raise NotConnectedError()

        self._clearbuffer()
        self._cmd = CmdCode.GET_MAC_ADDRESS
        self._send()
        mac = MacAddress()
        for i in range(6):
            mac[i] = self[i]
        return mac

    @property
    def name(self):
        return self.get_name()

    @name.setter
    def name(self, name: str):
        self.set_name(name)

    def get_name(self):
        if not self.connected:
            raise NotConnectedError()

        self._clearbuffer()
        self._cmd = CmdCode.GET_NAME
        self._send()
        if self._status == StatusCode.OK:
            return bytearray(filter(lambda x: x is not 0, self._buffer[2:])).decode("ascii")
        else:
            return None

    def set_name(self, name: str):
        if not self.connected:
            raise NotConnectedError()

        self._clearbuffer()
        self._cmd = CmdCode.SET_NAME
        if not name:
            raise ValueError("No name provided")

        if len(name) > 20:
            raise ValueError("Name to long. Max length is 20")

        self._string_to_buffer(name)
        self._send()

    def save_settings(self):
        if not self.connected:
            raise NotConnectedError()

        self._clearbuffer()
        self._cmd = CmdCode.SAVE_SETTINGS
        self._send()

    def erase_settings(self):
        if not self.connected:
            raise NotConnectedError()

        self._clearbuffer()
        self._cmd = CmdCode.ERASE_SETTINGS
        self._send()

    def reset_settings(self):
        if not self.connected:
            raise NotConnectedError()

        self._clearbuffer()
        self._cmd = CmdCode.RESET_SETTINGS
        self._send()

    @property
    def ssid(self) -> str:
        if not self.connected:
            raise NotConnectedError()

        self._clearbuffer()
        self._cmd = CmdCode.GET_SSID
        self._send()
        if self._status == StatusCode.OK:
            return self._buffer_to_string()
        else:
            return None

    @ssid.setter
    def ssid(self, val: str):
        if not self.connected:
            raise NotConnectedError()

        self._clearbuffer()
        if not val:
            self._cmd = CmdCode.CLEAR_SSID
        else:
            self._cmd = CmdCode.SET_SSID
            self._string_to_buffer(val)

        self._send()

    @property
    def wifi_flags(self) -> WiFiFlag:
        if not self.connected:
            raise NotConnectedError()

        self._clearbuffer()
        self._cmd = CmdCode.GET_WIFI_FLAGS
        self._send()
        return self[0] & (WiFiFlag.ALL)

    def clear_password(self):
        if not self.connected:
            raise NotConnectedError()

        self._clearbuffer()
        self._cmd = CmdCode.CLEAR_PASSWORD
        self._send()

    def set_password(self, password: str):
        if not self.connected:
            raise NotConnectedError()

        if not password:
            self.clear_password()
        else:
            self._clearbuffer()
            self._cmd = CmdCode.SET_PASSWORD
            self._string_to_buffer(password)
            self._send()

    def get_password(self):
        if not self.connected:
            raise NotConnectedError()

        self._clearbuffer()
        self._cmd = CmdCode.GET_PASSWORD
        self._send()
        if self._status == StatusCode.NOT_FOUND:
            return None

        if self._status == StatusCode.OK:
            return self._buffer_to_string()

        self._print_buffer()
        raise BadRequestError()

    def activate_wifi_sta(self):
        if not self.connected:
            raise NotConnectedError()

        self._clearbuffer()
        self._cmd = CmdCode.SET_WIFI_ACTIVE
        self[0] = 1
        self._send()

    @property
    def wifi_sta_active(self):
        return ((self.wifi_flags & WiFiFlag.FLAG_ACTIVE) == WiFiFlag.FLAG_ACTIVE)

    def deactivate_wifi_sta(self):
        if not self.connected:
            raise NotConnectedError()

        self._clearbuffer()
        self._cmd = CmdCode.SET_WIFI_ACTIVE
        self._send()

    @property
    def deep_sleep_seconds(self):
        if not self.connected:
            raise NotConnectedError()

        self._clearbuffer()
        self._cmd = CmdCode.GET_IDLE_TIME
        self._send()
        self._print_buffer()
        return ((self[0] << 8) | self[1])

    @deep_sleep_seconds.setter
    def deep_sleep_seconds(self, val: int):
        if not self.connected:
            raise NotConnectedError()

        self._clearbuffer()
        self._cmd = CmdCode.SET_IDLE_TIME

        self[0] = (val >> 8) & 255
        self[1] = val & 255
        self._print_buffer()
        self._send()


    def tare(self, nr_times: int):
        self._clearbuffer()
        self._cmd = CmdCode.TARE
        self[0] = (nr_times & 0xFF)
        print(self[0])
        self._print_buffer()
        self._send()
        self._print_buffer()
        offset = ((self[0] << 24) | (self[1] << 16) | (self[2] << 8) | self[3]) / 100
        factor = ((self[4] << 24) | (self[5] << 16) | (self[6] << 8) | self[7]) / 100
        return (offset, factor)

    def calibrate(self, ref_weight:int, count:int):
        self._clearbuffer()
        self._cmd = CmdCode.CALIBRATE
        self[0] = (ref_weight >> 8) & 0xFF
        self[1] = ref_weight & 0xFF
        self[2] = count & 0xFF
        self._send()
        self._print_buffer()
        offset = ((self[0] << 24) | (self[1] << 16) | (self[2] << 8) | self[3]) / 100
        factor = ((self[4] << 24) | (self[5] << 16) | (self[6] << 8) | self[7]) / 100
        return (offset, factor)

    @property
    def weight(self):
        self._clearbuffer()
        self._cmd = CmdCode.GET_WEIGHT
        self[0] = 1
        self._send()
        return ((self[0] << 24) | (self[1] << 16) | (self[2] << 8) | self[3]) / 100

    def to_dict(self):
        _d = {}
        _d["firmware_version"] = "{version[0]}.{version[1]}.{version[2]}".format(version=self.get_version())
        _d['device_name'] = self.get_name()
        _d['mac_address'] = str(self.get_mac_address())
        _d["ssid"] = self.ssid
        _d["password"] = self.get_password()
        _d["wifi_sta_enabled"] = self.wifi_sta_active
        _d["deep_sleep_seconds"] = self.deep_sleep_seconds
        _d['scale_offset'] = self.get_scale_offset()
        _d["scale_factor"] = self.get_scale_factor()
        return _d



with NeoBeeShell(host="192.168.178.72") as shell:   
    #with NeoBeeShell() as shell:

    #shell.name = "NeoBee.One"
    #shell.ssid = "RepeaterOben24"
    #shell.set_password("4249789363748310")
    #shell.deep_sleep_seconds = 30
    #shell.save_settings()
    #d = shell.to_dict()
    #print(shell.tare(1))
    #shell.set_scale_offset(61191.0)
    #shell.set_scale_factor(21.88)
    print(shell.weight)
    #print(shell.calibrate(2962,1))
    #print(json.dumps(d, ensure_ascii=True, indent=2))
    #shell.save_settings()
    