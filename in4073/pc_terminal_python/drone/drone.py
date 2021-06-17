from enum import Enum
import time
import threading
from queue import Queue
from typing import List
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
    def __init__(self, cli, serials: List[Serial], serial_idx):
        self.terminate = False

        self.cli = cli
        self.serials = serials
        self.active_serial = serial_idx
        for ser in self.serials:
            ser.add_command_handler(self.handle_command)

        self.mode = None
        self.phi = 0
        self.theta = 0
        self.psi = 0
        self.rpm0 = 0
        self.rpm1 = 0
        self.rpm2 = 0
        self.rpm3 = 0

        self.heartbeat_enabled = True
        self.heartbeat_seq = 0
        self.heartbeat_seq_prev = 0
        self.heartbeat_seq_ts = 0
        self.heartbeat_ack = 0
        self.heartbeat_ack_ts = 0
        self.heartbeat_rt_time = None
        self.heartbeat_freq = 5
        self.heartbeat_margin = 3
        self.heartbeat_ack_queue = Queue()

        # start the thread loop now
        self.thread = threading.Thread(target=self.__thread_function)
        self.thread.start()

    def send_command(self, command, idx=None):
        if idx is None:
            idx = self.active_serial

        self.serials[idx].send_command(command)

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
        elif command.type == CommandType.Heartbeat:
            self.heartbeat_ack_queue.put(command.get_data("argument"))
        elif command.type == CommandType.CurrentComms:
            self.__receive_current_comms(command.get_data("argument"))

    def __receive_current_comms(self, idx):
        if idx != self.active_serial:
            self.active_serial = idx
            self.cli.to_cli("[drone      ] changed to comm {}".format(idx))

    def __thread_function(self):
        while not self.terminate:
            while not self.heartbeat_ack_queue.empty():
                self.ack_heartbeat(self.heartbeat_ack_queue.get())

            if self.heartbeat_enabled and self.mode is not None:
                self.beat_heart()
                if self.get_heartbeat_distance() > self.heartbeat_margin:
                    self.cli.to_cli("[drone      ] heartbeat distance exceeded {}".format(self.heartbeat_margin))
                    self.mode = None
            time.sleep(1 / self.heartbeat_freq)

    def enable_heartbeat(self, enabled):
        self.heartbeat_enabled = enabled


    def get_angles(self):
        return self.phi, self.theta, self.psi

    def get_rpm(self):
        return self.rpm0, self.rpm1, self.rpm2, self.rpm3

    def change_mode(self, mode: FlightMode):
        command = Command(CommandType.SetOrQueryMode)
        command.set_data(argument=mode.value)

        self.send_command(command)

    def set_comms(self, idx):
        if self.mode is FlightMode.Safe:
            command = Command(CommandType.SetComms)
            command.set_data(argument=idx)

            self.send_command(command)
        else:
            self.cli.to_cli("[drone      ] cannot change comm while not in mode Safe (current mode = {})".format(self.mode.name))

    def set_control(self, yaw, pitch, roll, throttle):
        command = Command(CommandType.SetControl)
        command.set_data(argument=self.mode.value, yaw=yaw, pitch=pitch, roll=roll, throttle=throttle)

        self.send_command(command)

    def set_params(self, id, value):
        command = Command(CommandType.SetParam)
        command.set_data(argument=id, value=value)

        self.send_command(command)

    def __time_in_ms(self):
        return time.time() / 1000

    # difference between sent out heartbeat sequence number and acknowledge heartbeat sequence number
    def get_heartbeat_distance(self):
        seq = self.heartbeat_seq if (self.heartbeat_seq >= self.heartbeat_ack) else self.heartbeat_seq + 16
        return seq - self.heartbeat_ack

    def beat_heart(self):
        if self.heartbeat_seq_prev != self.heartbeat_ack:
            self.cli.to_cli("[drone      ] previous heartbeat not acknowledged ({} != {})".format(self.heartbeat_seq_prev, self.heartbeat_ack))

        command = Command(CommandType.Heartbeat)
        command.set_data(argument=int(self.heartbeat_seq))
        self.heartbeat_seq_prev = self.heartbeat_seq

        self.send_command(command)
        self.heartbeat_seq_ts = self.__time_in_ms()

        if self.heartbeat_seq == 15:
            self.heartbeat_seq = 0
        else:
            self.heartbeat_seq += 1

    def ack_heartbeat(self, ack):
        self.heartbeat_ack = ack
        self.heartbeat_ack_ts = self.__time_in_ms()

        # self.cli.to_cli("[drone] ACK heartbeat ({}, {})".format(self.heartbeat_seq_prev, self.heartbeat_ack))

        if self.heartbeat_seq_prev == self.heartbeat_ack:
            self.heartbeat_rt_time = (self.heartbeat_ack_ts - self.heartbeat_seq_ts)
            # self.cli.to_cli("[drone] Heartbeat RT Delay: {}us".format(self.heartbeat_rt_time * 1000))
        else:
            self.cli.to_cli("[drone      ] Heartbeat missed")

