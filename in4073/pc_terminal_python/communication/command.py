from enum import Enum
from communication.crc8 import crc8
from queue import Queue


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

        if self.type == CommandType.CurrentMode:
            self.args = ["argument"]

    def set_data(self, **kwargs):
        for key, value in kwargs.items():
            self.__set_datum(key, value)

    def __set_datum(self, key: str, value):
        if key in self.args:
            self.data[key] = value

        # TODO: raise exception

    def __str__(self):
        parts = []
        for key, value in self.data.items():
            parts.append("{}={}".format(key, value))

        return "Command(type={}, data=({}))".format(self.type.name, ", ".join(parts))

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


class SerialCommandDecoder:
    def __init__(self):
        self.buffer = []
        self.pos = 0
        self.data_len = 0

        self.commands = Queue()

    def append(self, byte):
        self.buffer.append(byte)

        if self.pos == 0:
            self.data_len = self.get_data_length_from_header(byte)
            self.pos += 1
        else:
            if self.data_len == 0:
                if crc8(self.buffer, self.pos) == byte:
                    self.extract_command()
                else:
                    print("CRC8 failed")
                    self.clear_buffer()
            else:
                self.data_len -= 1
                self.pos += 1

    def empty(self):
        return self.commands.empty()

    def get(self):
        return self.commands.get()

    def extract_command(self):
        header = self.buffer[0]
        type = CommandType(header >> 4)
        cmd = Command(type)

        if type == CommandType.CurrentMode:
            cmd.set_data(argument=(header & 0b1111))

        self.commands.put(cmd)
        self.clear_buffer()

    def clear_buffer(self):
        self.buffer.clear()
        self.pos = 0
        self.data_len = 0

    @staticmethod
    def get_data_length_from_header(header):
        type = (header >> 4)

        if type == CommandType.CurrentMode.value:
            return 0

        print("unknown header type..")
        return 0
