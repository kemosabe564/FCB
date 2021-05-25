from drone.pygame import pygame
import threading
import time

from drone.drone import Drone, FlightMode
from drone.joystick import Joystick, JoystickAxis
from drone.keyboard import Keyboard


class Controller:
    def __init__(self, drone: Drone, joystick: Joystick, keyboard: Keyboard):
        self.drone = drone
        self.joystick = joystick
        self.keyboard = keyboard

        self.keyboard.set_on_event(self.handle_keyboard_event)

        self.terminate = False

        self.offset_yaw = 0
        self.step = 0.05

        # start the thread loop now
        self.thread = threading.Thread(target=self.thread_function)
        self.thread.start()

    def inc_check_limits(self, value):
        if (value + self.step) < 1:
            value = value + self.step
        return value

    def dec_check_limits(self, value):
        if (value - self.step) > -1:
            value = value - self.step
        return value

    def handle_keyboard_event(self, event):
        if event.type == pygame.KEYDOWN:
            if event.key == pygame.K_q:  # yaw down
                self.offset_yaw = self.dec_check_limits(self.offset_yaw)
            if event.key == pygame.K_w:  # yaw up
                self.offset_yaw = self.inc_check_limits(self.offset_yaw)
            if event.key == pygame.K_0:  # safe mode
                self.drone.change_mode(FlightMode.Safe)
            if event.key == pygame.K_1:  # panic mode
                self.drone.change_mode(FlightMode.Panic)
            if event.key == pygame.K_2:  # manual mode
                if self.drone.mode == FlightMode.Safe and self.input_safe():
                    self.drone.change_mode(FlightMode.Manual)
                else:
                    print("NOT SAFE")
            if event.key == pygame.K_3:  # calibration
                # self.drone.change_mode(FlightMode.Calibrate)
                pass
            if event.key == pygame.K_4:  # yaw rate
                # self.drone.change_mode(FlightMode.Yaw)
                pass

    # TODO: This needs to be changed to limit to [-1,+1]
    # TODO: Check what this needs to be mapped to
    def update_inputs(self):
        self.input_roll = self.joystick.get_axis(JoystickAxis.Roll)
        self.input_pitch = self.joystick.get_axis(JoystickAxis.Pitch)
        self.input_yaw = self.joystick.get_axis(JoystickAxis.Yaw)
        # TODO: re-add trimming
        self.input_throttle = self.joystick.get_axis(JoystickAxis.Throttle)

    def at_deadpoint(self, x):
        if (x > 120) and (x < 134):
            return True
        return False

    def input_safe(self):
        self.update_inputs()
        return (self.input_throttle <= 10) and self.at_deadpoint(self.input_yaw) and self.at_deadpoint(self.input_pitch) and self.at_deadpoint(self.input_roll)

    def map(self, x):
        return int((x + 1) * 127)

    def thread_function(self):
        while not self.terminate:
            if self.joystick.available():
                if self.drone.mode == FlightMode.Manual:
                    self.update_inputs()

                    self.drone.set_control(self.input_roll, self.input_pitch, self.input_yaw, self.input_throttle)

            time.sleep(0.01)

    def stop(self):
        self.terminate = True

    def join(self):
        self.thread.join()
