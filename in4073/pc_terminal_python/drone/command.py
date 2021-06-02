from enum import Enum
from drone.crc8 import crc8
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
    Heartbeat = 0b1010


class Command:
    def __init__(self, type: CommandType):
        self.type = type
        self.data = {}

        if self.type == CommandType.SetOrQueryMode:
            self.args = ["argument"]

        if self.type == CommandType.CurrentMode:
            self.args = ["argument"]

        if self.type == CommandType.SetControl:
            self.args = ["argument", "yaw", "pitch", "roll", "throttle"]

        if self.type == CommandType.DebugMessage:
            self.args = ["argument", "message"]

        if self.type == CommandType.Heartbeat:
            self.args = ["argument"]

    def set_data(self, **kwargs):
        for key, value in kwargs.items():
            self.__set_datum(key, value)

    def __set_datum(self, key: str, value):
        if key in self.args:
            self.data[key] = value
        else:
            raise NameError("Command does not contain field '{}'".format(key))

    def __str__(self):
        parts = []
        for key, value in self.data.items():
            parts.append("{}={}".format(key, value))

        return "Command(type={}, data=({}))".format(self.type.name, ", ".join(parts))

    def get_data(self, key: str):
        if key in self.args:
            return self.data[key]

        raise NameError("Command does not contain field '{}'".format(key))

    def encode(self):
        buffer = bytearray()
        crc_len = 1

        if self.type == CommandType.SetOrQueryMode:
            buffer.append((self.type.value << 4) | (self.get_data("argument") & 0b1111))
        elif self.type == CommandType.SetControl:
            buffer.append((self.type.value << 4) | (self.get_data("argument") & 0b1111))
            buffer.append(self.get_data("yaw") & 0b11111111)
            buffer.append(self.get_data("pitch") & 0b11111111)
            buffer.append(self.get_data("roll") & 0b11111111)
            buffer.append(self.get_data("throttle") & 0b11111111)
            crc_len = 5
        elif self.type == CommandType.Heartbeat:
            buffer.append((self.type.value << 4) | (self.get_data("sequence") & 0b1111))

        buffer.append(crc8(buffer, crc_len))

        return buffer


class SerialCommandDecoder:
    def __init__(self):
        self.buffer = []
        self.data_len = 0
        self.ext_header = 0
        self.header = True

        self.commands = Queue()

    def append(self, byte):
        self.buffer.append(byte)
        pos = len(self.buffer)

        if self.header:
            if pos == 1:  # first byte is primary header byte
                self.ext_header = self.get_ext_header_size(byte)

            if self.ext_header == 0:  # if extended header length is zero then next bytes are already data bytes
                self.data_len = self.get_data_length_from_header(self.buffer[0:pos])
                self.header = False
            else:
                self.ext_header -= 1
        else:
            if self.data_len == 0:
                if crc8(self.buffer, len(self.buffer) - 1) == byte:
                    self.extract_command()
                else:
                    print("CRC8 failed")
                    self.clear_buffer()
            else:
                self.data_len -= 1

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
        elif type == CommandType.Heartbeat:
            cmd.set_data(argument=(header & 0b1111))
        elif type == CommandType.DebugMessage:
            message = bytearray()
            size = self.buffer[1]
            id = (header & 0b1111)
            for byte in self.buffer[2:(2 + size)]:
                message.append(byte)

            cmd.set_data(argument=id, message=message.decode('utf-8'))

        self.commands.put(cmd)
        self.clear_buffer()

    def clear_buffer(self):
        self.buffer.clear()
        self.header = True
        self.ext_header = 0
        self.data_len = 0

    @staticmethod
    def get_data_length_from_header(header):
        type = (header[0] >> 4)

        if type == CommandType.DebugMessage.value:
            return header[1]

        return 0

    @staticmethod
    def get_ext_header_size(header):
        type = (header >> 4)

        # currently only the debug message has an extended header size
        if type == CommandType.DebugMessage.value:
            return 1

        return 0
