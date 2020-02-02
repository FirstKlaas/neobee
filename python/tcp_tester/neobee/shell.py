from enum import IntEnum, IntFlag
import socket

from binascii import hexlify

from .error import (
    BadRequestError, AlreadyConnectedError, NotConnectedError, 
    NetworkError, WrongResponseCommandError)

from .util import HighByte, LowByte
from .net import MacAddress

__ALL__ = ["NeoBeeShell"]

class CmdCode(IntEnum):
    NOP = 0
    GET_NAME = 1
    SET_NAME = 2
    GET_FLAGS = 3
    RESET_SETTINGS = 4
    SAVE_SETTINGS = 5
    ERASE_SETTINGS = 6
    RESET_ESP = 7

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
    GET_MQTT_LOGIN = 34
    SET_MQTT_LOGIN = 35
    GET_MQTT_PASSWORD = 36,
    SET_MQTT_PASSWORD = 37,
    SET_MQTT_ACTIVE = 38
    GET_MQTT_FLAGS = 39

    GET_TEMPERATURE = 40
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


class StatusCode(IntEnum):
    NONE = (0,)
    OK = (1,)
    BAD_REQUEST = (2,)
    NOT_FOUND = (3,)
    ILLEGAL_STATE = 4


"""
def myreceive(sckt, handler=None, max_tries=10, delay=10):
    bytes_recd = 0
    buffer = bytearray(MSGLEN)
    number_of_tries = 0

    while bytes_recd < MSGLEN:
        chunk = sckt.recv(min(MSGLEN - bytes_recd, 32))
        if chunk == b"":
            raise RuntimeError("socket connection broken")
        chunksize = len(chunk)
        buffer[bytes_recd : bytes_recd + chunksize] = chunk
        bytes_recd = bytes_recd + len(chunk)

    response = Response(buffer[0], buffer[1], buffer[2:])
    if handler is not None:
        handler(response)
"""

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

        self.__exit__(None, None, None)

    def _clearbuffer(self):
        self._buffer[:] = [0] * 32

    def _buffer_to_string(self):
        return bytearray(
            filter(lambda x: x >= 32 and x <= 127, self._buffer[2:])
        ).decode("ascii")

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
            if chunk == b"":
                raise RuntimeError("socket connection broken")
            chunksize = len(chunk)
            self._buffer[bytes_recd : bytes_recd + chunksize] = chunk
            bytes_recd = bytes_recd + len(chunk)

    def _send(self):
        if not self.connected:
            raise NotConnectedError()

        request_cmd = self._cmd
        try:
            self._socket.send(self._buffer)
            self._receive()
        except:
            raise NetworkError()

        if request_cmd != self._cmd:
            raise WrongResponseCommandError()

        if self._status == StatusCode.BAD_REQUEST:
            raise BadRequestError()

    def _print_buffer(self):
        print(":".join("{:02x}".format(x) for x in self._buffer))

    def __getitem__(self, index):
        return self._buffer[index + 2]

    def __setitem__(self, index, value):
        self._buffer[index + 2] = value & 0xFF

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
        iValue = int(value * 100)
        self[0] = (iValue >> 24) & 0xFF
        self[1] = (iValue >> 16) & 0xFF
        self[2] = (iValue >> 8) & 0xFF
        self[3] = (iValue) & 0xFF
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
        iValue = int(value * 100)
        self[0] = (iValue >> 24) & 0xFF
        self[1] = (iValue >> 16) & 0xFF
        self[2] = (iValue >> 8) & 0xFF
        self[3] = (iValue) & 0xFF
        self._send()

    def get_mac_address(self):
        if not self.connected:
            raise NotConnectedError()

        self._clearbuffer()
        self._cmd = CmdCode.GET_MAC_ADDRESS
        self._send()
        mac = MacAddress([0]*6)
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
            return bytearray(filter(lambda x: x is not 0, self._buffer[2:])).decode(
                "ascii"
            )
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

    @property
    def wifi_password(self):
        self._clearbuffer()
        self._cmd = CmdCode.GET_PASSWORD
        self._send()
        if self._status == StatusCode.NOT_FOUND:
            return None

        if self._status == StatusCode.OK:
            return self._buffer_to_string()

        raise BadRequestError()

    @wifi_password.setter
    def wifi_password(self, password: str):
        if not self.connected:
            raise NotConnectedError()

        if not password:
            self.clear_password()
        else:
            self._clearbuffer()
            self._cmd = CmdCode.SET_PASSWORD
            self._string_to_buffer(password)
            self._send()

    def activate_wifi_sta(self):
        if not self.connected:
            raise NotConnectedError()

        self._clearbuffer()
        self._cmd = CmdCode.SET_WIFI_ACTIVE
        self[0] = 1
        self._send()

    @property
    def wifi_sta_active(self):
        return (self.wifi_flags & WiFiFlag.FLAG_ACTIVE) == WiFiFlag.FLAG_ACTIVE

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
        return (self[0] << 8) | self[1]

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

    @property
    def temperature(self):
        self._clearbuffer()
        self._cmd = CmdCode.GET_TEMPERATURE
        self._send()
        return ((self[0] << 24) | (self[1] << 16) | (self[2] << 8) | self[3]) / 100

    def tare(self, nr_times: int):
        self._clearbuffer()
        self._cmd = CmdCode.TARE
        self[0] = nr_times & 0xFF
        self._print_buffer()
        self._send()
        self._print_buffer()
        offset = ((self[0] << 24) | (self[1] << 16) | (self[2] << 8) | self[3]) / 100
        factor = ((self[4] << 24) | (self[5] << 16) | (self[6] << 8) | self[7]) / 100
        return (offset, factor)

    def calibrate(self, ref_weight: int, count: int):
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
    def mqtt_host(self):
        self._clearbuffer()
        self._cmd = CmdCode.GET_MQTT_HOST
        self._send()
        if self._status == StatusCode.NOT_FOUND:
            return None

        if self._status == StatusCode.OK:
            return self._buffer_to_string()

        raise BadRequestError()

    @mqtt_host.setter
    def mqtt_host(self, val):
        self._clearbuffer()
        self._cmd = CmdCode.SET_MQTT_HOST
        if val is not None:
            self._string_to_buffer(val)
        self._send()

    @property
    def mqtt_port(self):
        self._clearbuffer()
        self._cmd = CmdCode.GET_MQTT_PORT
        self._send()
        if self._status == StatusCode.NOT_FOUND:
            return None

        if self._status == StatusCode.OK:
            return (self[0] << 8) | self[1]

        raise BadRequestError()

    @mqtt_port.setter
    def mqtt_port(self, port:int):
        self._clearbuffer()
        self._cmd = CmdCode.SET_MQTT_PORT
        self[0] = HighByte(port)
        self[1] = LowByte(port)
        self._print_buffer()
        self._send()


    @property
    def mqtt_login(self):
        self._clearbuffer()
        self._cmd = CmdCode.GET_MQTT_LOGIN
        self._send()
        if self._status == StatusCode.NOT_FOUND:
            return None

        if self._status == StatusCode.OK:
            return self._buffer_to_string()

        raise BadRequestError()

    @mqtt_login.setter
    def mqtt_login(self, val):
        self._clearbuffer()
        self._cmd = CmdCode.SET_MQTT_LOGIN
        if val is not None:
            self._string_to_buffer(val)
        self._send()

    @property
    def mqtt_password(self):
        self._clearbuffer()
        self._cmd = CmdCode.GET_MQTT_PASSWORD
        self._send()
        if self._status == StatusCode.NOT_FOUND:
            return None

        if self._status == StatusCode.OK:
            return self._buffer_to_string()

        raise BadRequestError()

    @mqtt_password.setter
    def mqtt_password(self, val):
        self._clearbuffer()
        self._cmd = CmdCode.SET_MQTT_PASSWORD
        if val is not None:
            self._string_to_buffer(val)
        self._send()

    @property
    def weight(self):
        self._clearbuffer()
        self._cmd = CmdCode.GET_WEIGHT
        self[0] = 1
        self._send()
        return ((self[0] << 24) | (self[1] << 16) | (self[2] << 8) | self[3]) / 100

    def reset(self):
        self._clearbuffer()
        self._cmd = CmdCode.RESET_ESP
        self._send()

    def to_dict(self):
        _d = {}
        _d["firmware_version"] = "{version[0]}.{version[1]}.{version[2]}".format(
            version=self.get_version()
        )
        _d["device_name"] = self.get_name()
        _d["mac_address"] = str(self.get_mac_address())
        _d["ssid"] = self.ssid
        _d["password"] = self.wifi_password
        _d["wifi_sta_enabled"] = self.wifi_sta_active
        _d["deep_sleep_seconds"] = self.deep_sleep_seconds
        _d["scale_offset"] = self.get_scale_offset()
        _d["scale_factor"] = self.get_scale_factor()
        _d["mqtt_host"] = self.mqtt_host
        _d["mqtt_port"] = self.port
        _d["mqtt_login"] = self.mqtt_login
        _d["mqtt_password"] = self.mqtt_password

        return _d
