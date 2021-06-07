from enum import Enum

from drone.serial import Serial
from drone.command import Command, CommandType


class FlightMode(Enum):
    Invalid = 0
    Init = 1
    Safe = 2
    Panic = 3
    Manual = 4
    Calibrate = 5
    Yaw = 6
    Full = 7
    Raw = 8
    HoldHeight = 9


class Drone:
    def __init__(self, serial: Serial):
        self.serial = serial
        self.serial.add_command_handler(self.handle_command)

        self.mode = None
        self.phi = 0
        self.theta = 0
        self.psi = 0
        self.rpm0 = 0
        self.rpm1 = 0
        self.rpm2 = 0
        self.rpm3 = 0

    def handle_command(self, command):
        if type(command) != Command:
            return

        if command.type == CommandType.CurrentMode:
            self.mode = FlightMode(command.get_data("argument"))
        elif command.type == CommandType.CurrentTelemetry:
            self.phi = command.get_data("roll_angle")
            self.theta = command.get_data("pitch_angle")
            self.psi = command.get_data("yaw_angle")
            self.rpm0 = command.get_data("rpm0")
            self.rpm1 = command.get_data("rpm1")
            self.rpm2 = command.get_data("rpm2")
            self.rpm3 = command.get_data("rpm3")


    def get_angles(self):
        return self.phi, self.theta, self.psi

    def get_rpm(self):
        return self.rpm0, self.rpm1, self.rpm2, self.rpm3

    def change_mode(self, mode: FlightMode):
        command = Command(CommandType.SetOrQueryMode)
        command.set_data(argument=mode.value)

        self.serial.send_command(command)

    def set_control(self, yaw, pitch, roll, throttle):
        command = Command(CommandType.SetControl)
        command.set_data(argument=self.mode.value, yaw=yaw, pitch=pitch, roll=roll, throttle=throttle)

        self.serial.send_command(command)

    def set_params(self, id, value):
        command = Command(CommandType.SetParam)
        command.set_data(argument=id, value=value)

        self.serial.send_command(command)
