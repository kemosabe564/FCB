import threading
import serial
from typing import Callable


class CommandHandler:
    def __init__(self, port: str, baud: int, received_handler: Callable):
        self.port = port
        self.baud = baud
        self.received_handler = received_handler
        self.terminate = False

        self.thread = threading.Thread(target=self.thread_function)
        self.thread.start()

    def thread_function(self):
        with serial.Serial(self.port, self.baud) as ser:
            while not self.terminate:
                line = ser.readline()
                if line:
                    print(line)
                # self.received_handler(line)

    def stop(self):
        self.terminate = True

    def join(self):
        return self.thread.join()
