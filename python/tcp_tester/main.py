from enum import IntEnum, IntFlag
import socket
from binascii import hexlify

class CmdCode(IntEnum):
    NOP              =   0,
    GET_NAME         =   1,
    SET_NAME         =   2,
    GET_FLAGS        =   3,
    GET_SCALE_OFFSET =  10,
    SET_SCALE_OFFSET =  11,
    GET_SCALE_FACTOR =  12,
    SET_SCALE_FACTOR =  13,

    GET_MAC_ADDRESS  =  80,
    GET_VERSION      =  81,
    SET_IDLE_TIME    =  82,
    GET_IDLE_TIME    =  83, 
    TARE             = 200,
    CALIBRATE        = 201,
    GET_WEIGHT       = 202

class StatusFlag(IntFlag):

    FLAG_OFFSET_SET            1
    FLAG_FACTOR_SET            2
    FLAG_GAIN_SET              4
    FLAG_NAME_SET              8
    FLAG_ADDR_INSIDE_SET      16
    FLAG_ADDR_OUTSIDE_SET     32
    DEEP_SLEEP_SET            64
    WIFI_NETWORK_SET         128


class RequestFlag(IntFlag):
"""
Every request sends flags in the second
byte of the request.

These are the possible flags, which can
be mixed to fit to your need.
""""    
    AUTOSAVE_CTX               1

class StatusCode(IntEnum):
    OK = 20,
    BAD_REQUEST = 40,
    NOT_FOUND = 44

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

        response = Response(buffer[0],buffer[1],buffer[2:])
        if handler is not None:
            handler(response)

class MacAddress:

    def __init__(self):
        self._addr = bytearray(6)

    def __getitem__(self, index:int) -> int:
        return self._addr[index]

    def __setitem__(self, index:int, val:int):
        self._addr[index] = val

    def __str__(self):
        return ':'.join("{:02x}".format(x) for x in self._addr)

class NeoBeeShell:

    def __init__(self, host="192.168.4.1", port=8888):
        self._socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.host = host
        self.port = port
        self._buffer = bytearray(32)


    def __enter__(self):
        print(f"Connecting to {self.host}:{self.port}")
        self._socket.connect((self.host, self.port))
        return self

    def __exit__(self, type, value, traceback):
        print("Closing connection")
        self._socket.shutdown(1)
        self._socket.close()

    def _clearbuffer(self):
        for i in range(32):
            self._buffer[i] = 0

    def _receive(self):
        bytes_recd = 0        
        while bytes_recd < 32:
            chunk = self._socket.recv(min(32 - bytes_recd, 32))
            if chunk == b'':
                raise RuntimeError("socket connection broken")
            chunksize = len(chunk)
            self._buffer[bytes_recd:bytes_recd+chunksize] = chunk
            bytes_recd = bytes_recd + len(chunk)

    def _send(self):
        self._socket.send(self._buffer)
        self._receive()

    def _print_buffer(self):
        print( ':'.join("{:02x}".format(x) for x in self._buffer))

    def __getitem__(self, index):
        return self._buffer[index+2]

    def __setitem__(self, index, value):
        self._buffer[index+2] = (value & 0xff)

    def get_version(self):
        self._clearbuffer()
        self._cmd = CmdCode.GET_VERSION
        self._send()
        return (self[0],self[1],self[2])

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
        self._clearbuffer()
        self._cmd = CmdCode.SET_SCALE_OFFSET
        iValue = int(value*100)
        self[0] = (iValue >> 24) & 0xff
        self[1] = (iValue >> 16) & 0xff
        self[2] = (iValue >> 8) & 0xff
        self[3] = (iValue) & 0xff
        self._send()

    def get_scale_factor(self):
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
        self._clearbuffer()
        self._cmd = CmdCode.SET_SCALE_FACTOR
        iValue = int(value*100)
        self[0] = (iValue >> 24) & 0xff
        self[1] = (iValue >> 16) & 0xff
        self[2] = (iValue >> 8) & 0xff
        self[3] = (iValue) & 0xff
        self._send()

    def get_mac_address(self):
        self._clearbuffer()
        self._cmd = CmdCode.GET_MAC_ADDRESS
        self._send()
        mac = MacAddress()
        for i in range(6):
            mac[i] = self[i]
        return mac
    
    def get_name(self):
        self._clearbuffer()
        self._cmd = CmdCode.GET_NAME
        self._send()
        self._print_buffer()

    def set_name(self, name:str):
        self._clearbuffer()
        self._cmd = CmdCode.SET_NAME
        if not name:
            raise ValueError("No name provided")

        if len(name) > 20:
            raise ValueError("Name to long. Max length is 20")

        for index, char in enumerate(name):
            self[index] = ord(char)

        self._print_buffer()
        self._send()

with NeoBeeShell() as shell:
    print("Version: ", shell.get_version())
    print("Offset: ", shell.get_scale_offset())
    shell.set_scale_offset(666.66)
    print("Offset; ", shell.get_scale_offset())
    print("Factor: ", shell.get_scale_factor())
    shell.set_scale_factor(12.33)
    print("Factor: ", shell.get_scale_factor())
    print("MAc: ", shell.get_mac_address())
    shell.set_name('Klaas')
    shell.get_name()
