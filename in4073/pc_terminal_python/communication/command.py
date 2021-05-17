from enum import Enum
from communication.crc8 import crc8


class CommandType(Enum):
    Undefined = 0
    SetOrQueryMode = 0b0001
    CurrentMode = 0b0010
    SetControl = 0b0011
    AckControl = 0b0100
    QueryForces = 0b0101
    CurrentForces = 0b0110
    DebugMessage = 0b0111
    SetParam = 0b1000
    AckParam = 0b1001


class Command:
    def __init__(self, type: CommandType):
        self.type = type
        self.data = {}

        if self.type == CommandType.SetOrQueryMode:
            self.args = ["argument"]

    def set_data(self, **kwargs):
        for key, value in kwargs.items():
            self.__set_datum(key, value)

    def __set_datum(self, key: str, value):
        if key in self.args:
            self.data[key] = value

        # TODO: raise exception

    def get(self, key: str):
        if key in self.args:
            return self.data[key]

        return None  # TODO: raise exception

    def encode(self):
        buffer = bytearray()
        crc_len = 1

        if self.type == CommandType.SetOrQueryMode:
            buffer.append((self.type.value << 4) | (self.get("argument") & 0b1111))

        buffer.append(crc8(buffer, crc_len))

        return buffer
