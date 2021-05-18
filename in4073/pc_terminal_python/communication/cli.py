import threading
import serial
from typing import Callable
from enum import Enum
import re

from communication.command import Command, CommandType


class CLIAction(Enum):
    Exit = 1
    SendCommand = 2


class CLI:
    def __init__(self, action_handler: Callable):
        self.action_handler = action_handler
        self.terminate = False
        self.typing = False

        self.thread = threading.Thread(target=self.thread_function)
        self.thread.start()

    def thread_function(self):
        while not self.terminate:
            self.typing = False
            input("")

            self.typing = True
            match = re.search("(\\w+)(?:\\((.+)\\))?", input("> "))
            cmd = match.group(1)

            if cmd == 'help':
                self.print_usage()

            if cmd == 'exit':
                self.action_handler(CLIAction.Exit)
                self.stop()

            if cmd == 'chmode':
                command = Command(CommandType.SetOrQueryMode)
                command.set_data(argument=int(match.group(2)))

                self.action_handler(CLIAction.SendCommand, command)

    def to_cli(self, data):
        if not self.typing:
            print(data)

    @staticmethod
    def print_usage():
        print("""
exit = terminate application
cmode(int) = change drone flightmode
        """)

    def stop(self):
        self.terminate = True

    def join(self):
        return self.thread.join()
