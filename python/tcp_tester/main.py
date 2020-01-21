from enum import IntEnum
import socket
from binascii import hexlify

class CmdCode(IntEnum):
    NOP = 0,
    GET_NAME = 1,
    SET_NAME = 2,
    GET_SCALE_OFFSET = 10,
    SET_SCALE_OFFSET = 11,
    GET_MAC_ADDRESS = 80,
    ECHO = 255

class StatusCode(IntEnum):
    OK = 20,
    BAD_REQUEST = 40,
    NOT_FOUND = 44

ECHO = 255
MSGLEN = 32
buffer = bytearray(MSGLEN)

buffer[0] = CmdCode.GET_SCALE_OFFSET
buffer[1] = ord('A')
buffer[2] = ord('B')
buffer[3] = ord('C')
buffer[4] = ord('D')

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

    



def handleResponse(response: Response) -> None:
    print("Woohoo")

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

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:

    try:
        s.connect(("192.168.4.1", 8888))
        s.sendall(buffer)
        myreceive(s, handleResponse)
    except OSError:
        print("Fehler")

