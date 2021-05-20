import pygame
import threading

from drone.serial import Serial


class Drone:
    def __init__(self, serial: Serial):
        self.serial = serial

    def get_yaw(self):
        pass

    def change_mode(self, mode):
        pass

    def set_control(self, yaw, pitch, roll, throttle):
        pass
