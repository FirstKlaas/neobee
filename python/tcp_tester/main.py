from enum import IntEnum
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

class NeoBeeShell:

    def __init__(self, host="192.168.4.1", port=8888):
        self._socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self._host = host
        self._port = port
        self._buffer = bytearray(32)


    def __enter__(self):
        print("Connecting")
        self._socket.connect((self._host, self._port))
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
        self._socket.sendall(self._buffer)
        self._receive()

    def _print_buffer(self):
        print( ':'.join("{:02x}".format(x) for x in self._buffer))

    def get_version(self):
        self._clearbuffer()
        self._buffer[0] = CmdCode.GET_VERSION
        self._send()
        self._print_buffer()

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
        print("Get Scale Offset")
        self._print_buffer()
        if self._status == StatusCode.OK:
            return ((self._buffer[2] << 24) | (self._buffer[3] << 16) | (self._buffer[4] << 8) | self._buffer[5])
        elif self._status == StatusCode.NOT_FOUND:
            return None
        else:
            raise RuntimeError()

    def set_scale_offset(self, value: float):
        self._clearbuffer()
        self._cmd = CmdCode.SET_SCALE_OFFSET
        iValue = int(value*100)
        self._buffer[2] = (iValue >> 24) & 0xff
        self._buffer[3] = (iValue >> 16) & 0xff
        self._buffer[4] = (iValue >> 8) & 0xff
        self._buffer[5] = (iValue) & 0xff
        self._print_buffer
        self._send()
        self._print_buffer


with NeoBeeShell() as shell:
    #shell.get_version()
    #print(shell.get_scale_offset())
    shell.set_scale_offset(17.3)
    print(shell.get_scale_offset())
