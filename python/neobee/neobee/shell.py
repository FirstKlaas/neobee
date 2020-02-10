from enum import IntEnum, IntFlag
import socket

from binascii import hexlify

from .error import (
    BadRequestError,
    AlreadyConnectedError,
    NotConnectedError,
    NetworkError,
    WrongResponseCommandError,
    BadMethodError,
    DataError,
)

from .util import HighByte, LowByte
from .net import MacAddress
from .error import NeoBeeError

__ALL__ = ["NeoBeeShell"]


class CmdCode(IntEnum):
    NOP = 0
    NAME = 1
    RESET_SETTINGS = 4
    SAVE_SETTINGS = 5
    ERASE_SETTINGS = 6
    RESET_ESP = 7

    SCALE_OFFSET = 10
    SCALE_FACTOR = 12

    SSID = 20
    PASSWORD = 23

    MQTT_HOST = 30
    MQTT_PORT = 32
    MQTT_LOGIN = 34
    MQTT_PASSWORD = 36

    GET_TEMPERATURE = 40

    GET_MAC_ADDRESS = 80
    GET_VERSION = 81
    IDLE_TIME = 82

    TARE = 200
    CALIBRATE = 201
    GET_WEIGHT = 202


class StatusCode(IntEnum):
    NONE = 0
    OK = 1
    BAD_REQUEST = 2
    NOT_FOUND = 3
    ILLEGAL_STATE = 4
    BAD_METHOD = 5


class RequestMethod(IntEnum):
    NONE = 0
    GET = 1
    PUT = 2
    DELETE = 3


class NeoBeeShell:
    def __init__(self, host="192.168.4.1", port=8888):
        self._socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.host = host
        self.port = port
        self._buffer = bytearray(32)
        self._connected = False

    def __enter__(self):
        try:
            self._socket.connect((self.host, self.port))
        except OSError:
            raise NeoBeeError("Could not connect to hive")

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
            if chunk == b"":
                raise RuntimeError("socket connection broken")
            chunksize = len(chunk)
            self._buffer[bytes_recd : bytes_recd + chunksize] = chunk
            bytes_recd = bytes_recd + len(chunk)

    def _send(self):
        if not self.connected:
            raise NotConnectedError()

        requestcommand = self.command
        try:
            self._socket.send(self._buffer)
            self._receive()
        except:
            raise NetworkError()

        if requestcommand != self.command:
            raise WrongResponseCommandError()

        if self.status == StatusCode.BAD_REQUEST:
            raise BadRequestError()

        if self.status == StatusCode.BAD_METHOD:
            raise BadMethodError()

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
        self.command = CmdCode.GET_VERSION
        self._send()
        return (self[0], self[1], self[2])

    @property
    def method(self) -> RequestMethod:
        return self._buffer[1] & 3

    @method.setter
    def method(self, val: RequestMethod):
        self._buffer[1] = (self._buffer[1] & (~3)) | val

    @property
    def command(self):
        return self._buffer[0]

    @command.setter
    def command(self, value: CmdCode):
        self._buffer[0] = value

    @property
    def status(self):
        return self._buffer[1]

    @property
    def scale_offset(self):
        if not self.connected:
            raise NotConnectedError()

        self._clearbuffer()
        self.method = RequestMethod.GET
        self.command = CmdCode.SCALE_OFFSET
        self._send()

        if self.status == StatusCode.OK:
            return ((self[0] << 24) | (self[1] << 16) | (self[2] << 8) | self[3]) / 100
        elif self.status == StatusCode.NOT_FOUND:
            return None
        else:
            raise RuntimeError()

    @scale_offset.setter
    def scale_offset(self, value: float):
        if not self.connected:
            raise NotConnectedError()

        if value <= 0:
            raise BadRequestError("Offset must be a positive value.")

        self._clearbuffer()
        self.method = RequestMethod.PUT
        self.command = CmdCode.SCALE_OFFSET
        iValue = int(value * 100)
        self[0] = (iValue >> 24) & 0xFF
        self[1] = (iValue >> 16) & 0xFF
        self[2] = (iValue >> 8) & 0xFF
        self[3] = (iValue) & 0xFF
        self._send()

    @property
    def scale_factor(self):
        if not self.connected:
            raise NotConnectedError()

        self._clearbuffer()
        self.method = RequestMethod.GET
        self.command = CmdCode.SCALE_FACTOR
        self._send()
        if self.status == StatusCode.OK:
            return ((self[0] << 24) | (self[1] << 16) | (self[2] << 8) | self[3]) / 100
        elif self.status == StatusCode.NOT_FOUND:
            return None
        else:
            raise RuntimeError()

    @scale_factor.setter
    def scale_factor(self, value: float):
        if not self.connected:
            raise NotConnectedError()

        if value <= 0:
            raise BadRequestError("Factor must be a positive value.")

        self._clearbuffer()
        self.method = RequestMethod.PUT
        self.command = CmdCode.SCALE_FACTOR
        iValue = int(value * 100)
        self[0] = (iValue >> 24) & 0xFF
        self[1] = (iValue >> 16) & 0xFF
        self[2] = (iValue >> 8) & 0xFF
        self[3] = (iValue) & 0xFF
        self._send()

    @property
    def mac_address(self):
        if not self.connected:
            raise NotConnectedError()

        self._clearbuffer()
        self.method = RequestMethod.GET
        self.command = CmdCode.GET_MAC_ADDRESS
        self._send()
        mac = MacAddress([0] * 6)
        for i in range(6):
            mac[i] = self[i]
        return mac

    @property
    def name(self):
        if not self.connected:
            raise NotConnectedError()

        self._clearbuffer()
        self.method = RequestMethod.GET
        self.command = CmdCode.NAME
        self._send()
        if self.status == StatusCode.OK:
            return bytearray(filter(lambda x: x is not 0, self._buffer[2:])).decode("ascii")
        else:
            return None

    @name.setter
    def name(self, name: str):
        if not self.connected:
            raise NotConnectedError()

        self._clearbuffer()
        self.method = RequestMethod.PUT
        self.command = CmdCode.NAME
        if not name:
            self.method = RequestMethod.DELETE
        else:
            if len(name) > 20:
                raise DataError("Name to long. Max length is 20")
            self._string_to_buffer(name)

        self._send()

    def save_settings(self):
        if not self.connected:
            raise NotConnectedError()

        self._clearbuffer()
        self.command = CmdCode.SAVE_SETTINGS
        self._send()

    def erase_settings(self):
        if not self.connected:
            raise NotConnectedError()

        self._clearbuffer()
        self.command = CmdCode.ERASE_SETTINGS
        self._send()

    def reset_settings(self):
        if not self.connected:
            raise NotConnectedError()

        self._clearbuffer()
        self.command = CmdCode.RESET_SETTINGS
        self._send()

    @property
    def ssid(self) -> str:
        if not self.connected:
            raise NotConnectedError()

        self._clearbuffer()
        self.method = RequestMethod.GET
        self.command = CmdCode.SSID
        self._send()
        if self.status == StatusCode.OK:
            return self._buffer_to_string()
        else:
            return None

    @ssid.setter
    def ssid(self, val: str):
        if not self.connected:
            raise NotConnectedError()

        self._clearbuffer()
        self.command = CmdCode.SSID
        if not val:
            self.method = RequestMethod.DELETE
        else:
            self.method = RequestMethod.PUT
            self._string_to_buffer(val)

        self._send()

    @property
    def wifi_password(self):
        self._clearbuffer()
        self.method = RequestMethod.GET
        self.command = CmdCode.PASSWORD
        self._send()
        if self.status == StatusCode.NOT_FOUND:
            return None

        if self.status == StatusCode.OK:
            return self._buffer_to_string()

        raise BadRequestError()

    @wifi_password.setter
    def wifi_password(self, password: str):
        if not self.connected:
            raise NotConnectedError()

        self._clearbuffer()
        self.command = CmdCode.PASSWORD

        if not password:
            self.method = RequestMethod.DELETE
        else:
            self.method = RequestMethod.PUT
            self._string_to_buffer(password)
            self._send()

    @property
    def deep_sleep_seconds(self):
        if not self.connected:
            raise NotConnectedError()

        self._clearbuffer()
        self.method = RequestMethod.GET
        self.command = CmdCode.IDLE_TIME
        self._send()
        return (self[0] << 8) | self[1]

    @deep_sleep_seconds.setter
    def deep_sleep_seconds(self, val: int):
        if not self.connected:
            raise NotConnectedError()

        self._clearbuffer()
        self.method = RequestMethod.PUT
        self.command = CmdCode.IDLE_TIME

        self[0] = (val >> 8) & 255
        self[1] = val & 255
        self._send()

    @property
    def temperature(self):
        self._clearbuffer()
        self.method = RequestMethod.GET
        self.command = CmdCode.GET_TEMPERATURE
        self._send()
        return ((self[0] << 24) | (self[1] << 16) | (self[2] << 8) | self[3]) / 100

    def tare(self, nr_times: int):
        self._clearbuffer()
        self.method = RequestMethod.NONE
        self.command = CmdCode.TARE
        self[0] = nr_times & 0xFF
        self._send()
        offset = ((self[0] << 24) | (self[1] << 16) | (self[2] << 8) | self[3]) / 100
        factor = ((self[4] << 24) | (self[5] << 16) | (self[6] << 8) | self[7]) / 100
        return (offset, factor)

    def calibrate(self, ref_weight: int, count: int):
        print("Calibrating", count, "-times")
        self._clearbuffer()
        self.method = RequestMethod.GET
        self.command = CmdCode.CALIBRATE
        self[0] = (ref_weight >> 8) & 0xFF
        self[1] = ref_weight & 0xFF
        self[2] = count & 0xFF
        self._print_buffer()
        self._send()
        offset = ((self[0] << 24) | (self[1] << 16) | (self[2] << 8) | self[3]) / 100
        factor = ((self[4] << 24) | (self[5] << 16) | (self[6] << 8) | self[7]) / 100
        return (offset, factor)

    @property
    def mqtt_host(self):
        self._clearbuffer()
        self.method = RequestMethod.GET
        self.command = CmdCode.MQTT_HOST
        self._send()
        if self.status == StatusCode.NOT_FOUND:
            return None

        if self.status == StatusCode.OK:
            return self._buffer_to_string()

        raise BadRequestError()

    @mqtt_host.setter
    def mqtt_host(self, val):
        self._clearbuffer()
        self.method = RequestMethod.PUT
        self.command = CmdCode.MQTT_HOST
        if val is not None:
            self._string_to_buffer(val)
        self._send()

    @property
    def mqtt_port(self):
        self._clearbuffer()
        self.method = RequestMethod.GET
        self.command = CmdCode.MQTT_PORT
        self._send()
        if self.status == StatusCode.NOT_FOUND:
            return None

        if self.status == StatusCode.OK:
            return (self[0] << 8) | self[1]

        raise BadRequestError()

    @mqtt_port.setter
    def mqtt_port(self, port: int):
        self._clearbuffer()
        self.method = RequestMethod.PUT
        self.command = CmdCode.MQTT_PORT
        self[0] = HighByte(port)
        self[1] = LowByte(port)
        self._send()

    @property
    def mqtt_login(self):
        self._clearbuffer()
        self.method = RequestMethod.GET
        self.command = CmdCode.MQTT_LOGIN
        self._send()
        if self.status == StatusCode.NOT_FOUND:
            return None

        if self.status == StatusCode.OK:
            return self._buffer_to_string()

        raise BadRequestError()

    @mqtt_login.setter
    def mqtt_login(self, val):
        self._clearbuffer()
        self.method = RequestMethod.PUT
        self.command = CmdCode.MQTT_LOGIN
        if val is not None:
            self._string_to_buffer(val)
        self._send()

    @property
    def mqtt_password(self):
        self._clearbuffer()
        self.method = RequestMethod.GET
        self.command = CmdCode.MQTT_PASSWORD
        self._send()
        if self.status == StatusCode.NOT_FOUND:
            return None

        if self.status == StatusCode.OK:
            return self._buffer_to_string()

        raise BadRequestError()

    @mqtt_password.setter
    def mqtt_password(self, val):
        self._clearbuffer()
        self.method = RequestMethod.PUT
        self.command = CmdCode.MQTT_PASSWORD
        if val is not None:
            self._string_to_buffer(val)
        self._send()

    @property
    def weight(self):
        self._clearbuffer()
        self.method = RequestMethod.GET
        self.command = CmdCode.GET_WEIGHT
        self[0] = 1
        self._send()
        return ((self[0] << 24) | (self[1] << 16) | (self[2] << 8) | self[3]) / 100

    def reset(self):
        self._clearbuffer()
        self.method = RequestMethod.NONE
        self.command = CmdCode.RESET_ESP
        self._send()

    def to_dict(self):
        _d = {}
        _d["firmware_version"] = "{version[0]}.{version[1]}.{version[2]}".format(
            version=self.get_version()
        )
        _d["device_name"] = self.name
        _d["mac_address"] = str(self.mac_address)
        _d["ssid"] = self.ssid
        _d["password"] = self.wifi_password
        _d["deep_sleep_seconds"] = self.deep_sleep_seconds
        _d["scale_offset"] = self.scale_offset
        _d["scale_factor"] = self.scale_factor
        _d["mqtt_host"] = self.mqtt_host
        _d["mqtt_port"] = self.mqtt_port
        _d["mqtt_login"] = self.mqtt_login
        _d["mqtt_password"] = self.mqtt_password

        return _d