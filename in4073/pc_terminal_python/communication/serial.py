import threading
import queue
import serial
import time
from typing import Callable

from communication.command import Command, SerialCommandDecoder


class Serial:
    def __init__(self, port: str, baud: int, command_handler: Callable):
        self.port = port
        self.baud = baud
        self.command_handler = command_handler
        self.terminate = False

        self.ascii_buffer = bytearray()
        self.protocol_enabled = False
        self.protocol_enable_c = 0
        self.decoder = SerialCommandDecoder()
        self.send_queue = queue.Queue()

        self.serial = serial.Serial(self.port, self.baud, timeout=1)

        # separate send and receive threads since pyserial busywaits for characters
        self.send_thread = threading.Thread(target=self.send_thread_function)
        self.send_thread.start()

        self.receive_thread = threading.Thread(target=self.receive_thread_function)
        self.receive_thread.start()

    def send_thread_function(self):
        with self.serial as ser:
            while not self.terminate:
                if not self.send_queue.empty():
                    command = self.send_queue.get()
                    ser.write(command.encode())

                    print(">> {}".format(command))

                time.sleep(0.01)

    def receive_thread_function(self):
        with self.serial as ser:
            while not self.terminate:
                # At first the MCU just spits out initialisation information
                # only after ten 0xFF characters have been sent do we switch
                # over to the protocol
                for byte in ser.read():
                    if not self.protocol_enabled:
                        self.__handle_ascii_data(byte)
                    else:
                        self.__handle_protocol_data(byte)

                time.sleep(0.01)

    def set_protocol(self, enabled):
        if enabled:
            print('Protocol enabled')
            self.ascii_buffer.clear()
        self.protocol_enabled = enabled

    def __handle_ascii_data(self, byte):
        self.ascii_buffer.append(byte)

        # protocol enabled logic
        if byte == 0xFF:
            self.protocol_enable_c += 1
        else:
            self.protocol_enable_c = 0

        if self.protocol_enable_c == 10:
            self.set_protocol(True)
            self.ascii_buffer.clear()
            return

        if byte == 10:  # ASCII \n = 10
            self.command_handler(self.ascii_buffer)
            self.ascii_buffer.clear()

    def __handle_protocol_data(self, byte):
        self.decoder.append(byte)

        while not self.decoder.empty():
            command = self.decoder.get()
            self.command_handler(command)
            print("<< {}".format(command))

    def send_command(self, command: Command):
        self.send_queue.put(command)

    def stop(self):
        self.terminate = True

    def join(self):
        self.send_thread.join()
        self.receive_thread.join()
