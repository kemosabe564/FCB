from enum import Enum

from drone.serial import Serial
from drone.command import Command, CommandType


class FlightMode(Enum):
    Init = 1
    Safe = 2
    Panic = 3
    Manual = 4
    Calibrate = 5
    Full = 6
    Raw = 7
    HoldHeight = 8


class Drone:
    def __init__(self, serial: Serial):
        self.serial = serial
        # self.serial.add_command_handler(self.handle_command)

        self.mode = None
        self.phi = 0
        self.theta = 0
        self.psi = 0

    def handle_command(self, command: Command):
        if command.type == CommandType.CurrentMode:
            self.mode = FlightMode(command.get_data("argument"))

    def get_angles(self):
        return self.phi, self.theta, self.psi

    def change_mode(self, mode):
        pass

    def set_control(self, yaw, pitch, roll, throttle):
        pass
