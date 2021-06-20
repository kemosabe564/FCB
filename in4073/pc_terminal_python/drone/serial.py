import threading
import queue
import serial
import time
from typing import Callable

from drone.command import Command, SerialCommandDecoder

#authored by Nathan
class Serial:
    def __init__(self, cli, port: str, baud: int, command_handler: Callable = None, idx=None):
        self.cli = cli
        self.port = port
        self.baud = baud
        self.idx = idx
        self.command_handlers = []
        if command_handler is not None:
            self.add_command_handler(command_handler)

        self.ascii_buffer = bytearray()
        self.protocol_enabled = False
        self.protocol_enable_c = 0
        self.decoder = SerialCommandDecoder()
        self.send_queue = queue.Queue()

        self.print_traffic = False
        self.failed_message = False

        self.serial = None # serial.Serial(self.port, self.baud, timeout=1)

        self.terminate = False
        # separate send and receive threads since pyserial busywaits for characters
        self.send_thread = threading.Thread(target=self.send_thread_function)
        self.send_thread.start()

        self.receive_thread = threading.Thread(target=self.receive_thread_function)
        self.receive_thread.start()
    #authored by Nathan
    def is_open(self):
        return (self.serial is not None) and self.serial.isOpen()
    #authored by Nathan
    def open_port(self):
        try:
            self.serial = serial.Serial(self.port, self.baud, timeout=1)
            self.cli.to_cli("[serial {}   ] port successfully opened".format(self.idx))
            self.failed_message = False
            return True
        except serial.SerialException:
            if not self.failed_message:
                self.cli.to_cli("[serial {}   ] could not open port".format(self.idx))
                self.failed_message = True

        return False
    #authored by Nathan
    def close_port(self):
        if self.serial:
            self.serial.close()
            self.cli.to_cli("[serial {}   ] closing port".format(self.idx))
    #authored by Nathan
    def send_thread_function(self):
        while not self.terminate:
            if not self.is_open():
                time.sleep(1)
            else:
                command = self.send_queue.get()
                buffer = command.encode()
                if self.print_traffic:
                    self.cli.to_cli("[serial {}   ] --> {} (raw = {})".format(self.idx, command, buffer))
                try:
                    self.serial.write(buffer)
                    self.serial.flush()
                except serial.SerialException:
                    self.close_port()
    #authored by Nathan
    def receive_thread_function(self):
        while not self.terminate:
            if not self.is_open() and not self.open_port():
                time.sleep(1)
            else:
                # At first the MCU just spits out initialisation information
                # only after ten 0xFF characters have been sent do we switch
                # over to the protocol
                try:
                    for byte in self.serial.read():
                        if not self.protocol_enabled:
                            self.__handle_ascii_data(byte)
                        else:
                            self.__handle_protocol_data(byte)
                except serial.SerialException:
                    self.close_port()
    #authored by Nathan
    def add_command_handler(self, handler: Callable):
        self.command_handlers.append(handler)
    #authored by Nathan
    def __dispatch_command(self, command: Command):
        for handler in self.command_handlers:
            handler(command)
    #authored by Nathan
    def set_protocol(self, enabled):
        if enabled:
            self.cli.to_cli("[serial {}   ] Protocol enabled".format(self.idx))
            self.ascii_buffer.clear()
        self.protocol_enabled = enabled
    #authored by Nathan
    def set_print_traffic(self, enabled):
        if enabled:
            self.cli.to_cli("[serial {}   ] Printing traffic enabled".format(self.idx))
        self.print_traffic = enabled
    #authored by Nathan
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
            self.__dispatch_command(self.ascii_buffer)
            self.ascii_buffer.clear()
    #authored by Nathan
    def __handle_protocol_data(self, byte):
        self.decoder.append(byte)

        while not self.decoder.empty():
            command = self.decoder.get()
            if self.print_traffic:
                self.cli.to_cli("[serial {}   ] <-- {} (raw = {})".format(self.idx, command, command.encode()))
            self.__dispatch_command(command)
    #authored by Nathan
    def send_command(self, command: Command):
        self.send_queue.put(command)
    #authored by Nathan
    def stop(self):
        self.terminate = True
    #authored by Nathan
    def join(self):
        self.send_thread.join()
        self.receive_thread.join()
