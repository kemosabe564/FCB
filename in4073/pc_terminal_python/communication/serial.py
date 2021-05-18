import threading
import queue
import serial
from typing import Callable

from communication.command import Command


class Serial:
    def __init__(self, port: str, baud: int, command_handler: Callable):
        self.port = port
        self.baud = baud
        self.command_handler = command_handler
        self.terminate = False

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

    def receive_thread_function(self):
        with self.serial as ser:
            while not self.terminate:
                b = ser.readline()
                if b:
                    self.command_handler(b)

    def send_command(self, command: Command):
        self.send_queue.put(command)

    def stop(self):
        self.terminate = True

    def join(self):
        self.send_thread.join()
        self.receive_thread.join()
