import threading
import serial
from typing import Callable
from enum import Enum
from queue import Queue
import re

from drone.command import Command, CommandType


class CLIAction(Enum):
    Exit = 1
    SendCommand = 2
    SetProtocol = 3


class CLI:
    def __init__(self, action_handler: Callable):
        self.action_handler = action_handler
        self.terminate = False

        self.typing = False
        self.output_q = Queue()

        self.thread = threading.Thread(target=self.thread_function)
        self.thread.start()

    def thread_function(self):
        self.__print_welcome_message()

        while not self.terminate:
            if not self.typing:
                self.__flush_output_queue()
                input("")
                self.typing = True

            else:
                string = input("> ").strip()  # remove leading and trailing whitespaces
                if string != "":
                    match = re.search("(\\w+)(?:\\((.+)\\))?", string)
                    cmd = match.group(1)

                    if cmd == 'help':
                        self.print_usage()
                    elif cmd == 'exit':
                        self.action_handler(CLIAction.Exit)
                        self.stop()
                    elif cmd == 'cmode':
                        command = Command(CommandType.SetOrQueryMode)
                        command.set_data(argument=int(match.group(2)))

                        self.action_handler(CLIAction.SendCommand, command)
                    elif cmd == 'qmode':
                        command = Command(CommandType.SetOrQueryMode)
                        command.set_data(argument=0)

                        self.action_handler(CLIAction.SendCommand, command)
                    elif cmd == 'setcontrol':
                        command = Command(CommandType.SetControl)
                        command.set_data(argument=0, yaw=1, pitch=2, roll=3, throttle=4)

                        self.action_handler(CLIAction.SendCommand, command)
                    elif cmd == 'protocol':
                        if match.group(2) == 'true':
                            self.action_handler(CLIAction.SetProtocol, True)
                        elif match.group(2) == 'false':
                            self.action_handler(CLIAction.SetProtocol, False)
                    else:
                        self.__print("Unknown command")
                        self.print_usage()

                self.typing = False

    def __print(self, *args):
        print(*args)

    def __print_welcome_message(self):
        self.__print("""==== Drone CLI ====
press enter to type command

type 'help' for... help""")

    def to_cli(self, data):
        if not self.typing:
            self.__print(data)
        else:
            self.output_q.put(data)

    def __flush_output_queue(self):
        while not self.output_q.empty():
            print(self.output_q.get())

    @staticmethod
    def print_usage():
        print("""
exit = terminate application
cmode(int) = change drone flightmode
qmode = query mode
protocol(boolean) = enable or disable the protocol manually
        """)

    def stop(self):
        if not self.terminate:
            print("Exiting application...")
            self.terminate = True

    def join(self):
        return self.thread.join()
