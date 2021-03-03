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
    INFO = 8

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


class NeoBeeInfoFlag(IntFlag):

    NONE = 0
    NAME = 1 << 0
    WIFI_SSID = 1 << 1
    WIFI_PASSWORD = 1 << 2
    MQTT_HOSTNAME = 1 << 3
    MQTT_PORT = 1 << 4
    MQTT_LOGIN = 1 << 5
    MQTT_PASSWORD = 1 << 6
    SCALE_OFFSET = 1 << 8
    SCALE_FACTOR = 1 << 9
    SCALE_GAIN = 1 << 10


class NeoBeeInfo:
    def __init__(self):
        self.major_version = None
        self.minor_version = None
        self.build_version = None
        self.flags = NeoBeeInfoFlag.NONE
        self.number_of_temperature_sensors = None
        self.scale_offset = None
        self.scale_factor = None

    @property
    def name_set(self) -> bool:
        return bool(self.flags & NeoBeeInfoFlag.NAME)

    @property
    def wifi_ssid_set(self) -> bool:
        return bool(self.flags & NeoBeeInfoFlag.WIFI_SSID)

    @property
    def wifi_password_set(self) -> bool:
        return bool(self.flags & NeoBeeInfoFlag.WIFI_PASSWORD)

    @property
    def mqtt_host_set(self) -> bool:
        return bool(self.flags & NeoBeeInfoFlag.MQTT_HOSTNAME)

    @property
    def mqtt_port_set(self) -> bool:
        return bool(self.flags & NeoBeeInfoFlag.MQTT_PORT)

    @property
    def mqtt_login_set(self) -> bool:
        return bool(self.flags & NeoBeeInfoFlag.MQTT_LOGIN)

    @property
    def mqtt_password_set(self) -> bool:
        return bool(self.flags & NeoBeeInfoFlag.MQTT_PASSWORD)

    @property
    def scale_offset_set(self) -> bool:
        return bool(self.flags & NeoBeeInfoFlag.SCALE_OFFSET)

    @property
    def scale_factor_set(self) -> bool:
        return bool(self.flags & NeoBeeInfoFlag.SCALE_FACTOR)

    @property
    def scale_gain_set(self) -> bool:
        return bool(self.flags & NeoBeeInfoFlag.SCALE_GAIN)


class NeoBeeShell:
    """
    The NeoBeeShell is the main class to configure the controller
    programmatically. The class implements pythons context protocol.
    To use this class, you have to be in the same network as the
    controller. When the controller acts as an access point, connect
    to the controller before using the class.

    When using with the ``with`` clause, the class connects before
    executing the statements and disconnects at the end. Even if
    an exception occurs. This is the preferred way to use the class.

    :param host: The IP of the controller. Defaults to 192.168.4.1
    :type host: str, optional
    :param port: The port to connect to. Defaults to 8888
    :type port: int
    """

    def __init__(self, host="192.168.4.1", port=8888):
        """
        This is the constructor
        """
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

    @property
    def connected(self):
        """
        Boolean flag indicating if the controller is connected or not.
        """
        return self._connected

    def connect(self):
        """
        Connects to the controller. If a connection already has been
        established, an AlreadyConnectedError is raised.

        The preferred way to use this class is in a `with` statement,
        which eliminates the need to connect or disconnect explicitly.
        """
        if self.connected:
            raise AlreadyConnectedError()

        self.__enter__()

    def disconnect(self):
        """
        Disconnects from the controller. If no connection has been
        established, an NotConnectedError is raised.
        """
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
            raise NotConnectedError("Not connected")

        requestcommand = self.command
        try:
            self._socket.send(self._buffer)
            self._receive()
        except:
            raise NetworkError("Network error")

        if requestcommand != self.command:
            raise WrongResponseCommandError("Response command does not match request command.")

        if self.status == StatusCode.BAD_REQUEST:
            raise BadRequestError("Bad request")

        if self.status == StatusCode.BAD_METHOD:
            raise BadMethodError("Bad method")

    def _print_buffer(self):
        print(":".join("{:02x}".format(x) for x in self._buffer))

    def __getitem__(self, index):
        return self._buffer[index + 2]

    def __setitem__(self, index, value):
        self._buffer[index + 2] = value & 0xFF

    @property
    def version(self):
        """
        Returns the version of the firmware as a tuple
        (MAJOR, MINOR, BUILD).
        """
        if not self.connected:
            raise NotConnectedError()

        self._clearbuffer()
        self.command = CmdCode.GET_VERSION
        self._send()
        return (self[0], self[1], self[2])

    def testBit(self, value, bitpos):
        if bitpos < 0 or bitpos > 7:
            raise ValueError("Bitpos must be within range 0 and 7")

        return (value >> bitpos) & 1 == 1

    @property
    def info(self) -> NeoBeeInfo:
        """
        Returns a `NeoBeeInfo` object, which contains all the current
        settings.
        """
        if not self.connected:
            raise NotConnectedError()

        self._clearbuffer()
        self.command = CmdCode.INFO
        self.method = RequestMethod.GET
        self._send()

        info = NeoBeeInfo()
        info.major_version = self[0]
        info.minor_version = self[1]
        info.build_version = self[2]

        info.flags = (self[4] << 8) | self[3]  # Read WORD (16bit)
        info.number_of_temperature_sensors = self[5]
        info.scale_offset = self._read_float(6) if info.scale_offset_set else None
        info.scale_factor = self._read_float(10) if info.scale_factor_set else None

        return info

    def _read_float(self, index: int = 0) -> float:
        return (
            (self[index] << 24) | (self[index + 1] << 16) | (self[index + 2] << 8) | self[index + 3]
        ) / 100.0

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
        """
        Float property for the offset of the scale.
        """
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
        """
        Float property for the scale factor.
        """
        if not self.connected:
            raise NotConnectedError("Not connected")

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
            raise NotConnectedError("Not connected")

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
        """
        Returns a MacAddress object with the mac address
        of the board.
        """
        if not self.connected:
            raise NotConnectedError("Not connected")

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
        """
        String property for the name of the board.
        The length of the name is limited to 20 and
        is expected to be in ascii encoded.

        If a name longer then 20 characters is provided,
        a DataError is raised.
        """
        if not self.connected:
            raise NotConnectedError("Not connected")

        self._clearbuffer()
        self.method = RequestMethod.GET
        self.command = CmdCode.NAME
        self._send()
        if self.status == StatusCode.OK:
            return bytearray(filter(lambda x: x != 0, self._buffer[2:])).decode("ascii")
        else:
            return None

    @name.setter
    def name(self, name: str):
        if not self.connected:
            raise NotConnectedError("Not connected")

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
        """
        Stores the settings to the flash memory.
        """
        if not self.connected:
            raise NotConnectedError("Not connected")

        self._clearbuffer()
        self.command = CmdCode.SAVE_SETTINGS
        self._send()

    def erase_settings(self):
        """
        Removes the stored settings from flash memory.
        After a reboot, the board acts as a AP again.
        """
        if not self.connected:
            raise NotConnectedError("Not connected")

        self._clearbuffer()
        self.command = CmdCode.ERASE_SETTINGS
        self._send()

    def reset_settings(self):
        if not self.connected:
            raise NotConnectedError("Not connected")

        self._clearbuffer()
        self.command = CmdCode.RESET_SETTINGS
        self._send()

    @property
    def ssid(self) -> str:
        """
        String property for the ssid of the wifi network.
        """
        if not self.connected:
            raise NotConnectedError("Not connected")

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
            raise NotConnectedError("Not connected")

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
        """
        String property for the wifi_password.
        """
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
            raise NotConnectedError("Not connected")

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
            raise NotConnectedError("Not connected")

        self._clearbuffer()
        self.method = RequestMethod.GET
        self.command = CmdCode.IDLE_TIME
        self._send()
        return (self[0] << 8) | self[1]

    @deep_sleep_seconds.setter
    def deep_sleep_seconds(self, val: int):
        if not self.connected:
            raise NotConnectedError("Not connected")

        self._clearbuffer()
        self.method = RequestMethod.PUT
        self.command = CmdCode.IDLE_TIME

        self[0] = (val >> 8) & 255
        self[1] = val & 255
        self._send()

    @property
    def temperature(self):
        """
        Reads the temperature of both sensors. Returns the temperature
        as a float value in celsius degree.
        """
        self._clearbuffer()
        self.method = RequestMethod.GET
        self.command = CmdCode.GET_TEMPERATURE
        self._send()
        return ((self[0] << 24) | (self[1] << 16) | (self[2] << 8) | self[3]) / 100

    def tare(self, nr_times: int):
        """
        Command to trigger the taring.

        Taring is the process to determine the 0 value of
        the scale. So, before triggering this command, empty
        the scale.

        nr_times specifies the number of measurements excecuted to
        calculate the tare as an average of the measured values.
        """
        self._clearbuffer()
        self.method = RequestMethod.NONE
        self.command = CmdCode.TARE
        self[0] = nr_times & 0xFF
        self._send()
        offset = ((self[0] << 24) | (self[1] << 16) | (self[2] << 8) | self[3]) / 100
        factor = ((self[4] << 24) | (self[5] << 16) | (self[6] << 8) | self[7]) / 100
        return (offset, factor)

    def calibrate(self, ref_weight: int, count: int):
        """
        Calibrates the scale. Before calibrating the scale, be sure,
        to tare the scale correctly.

        To calibrate the weight, a reference weight is needed. And the value
        you provide determines the unit to be used.

        Example:

        Put a weight of 1 kilogramm onto the scale. If you now call this method
        with a ref_weight of 1, all measurements are done in kilogramm. If you call
        this method with a value of 1000, all measurements are done in gramm.
        """
        if not self.scale_offset:
            raise BadRequestError("Scale not tared or not present.")

        self._clearbuffer()
        self.method = RequestMethod.GET
        self.command = CmdCode.CALIBRATE
        self[0] = (ref_weight >> 8) & 0xFF
        self[1] = ref_weight & 0xFF
        self[2] = count & 0xFF
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
            version=self.version
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
