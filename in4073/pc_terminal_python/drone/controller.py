from drone.pygame import pygame
import threading
import time

from drone.drone import Drone, FlightMode
from drone.joystick import Joystick, JoystickAxis, JoystickButton
from drone.keyboard import Keyboard


class Controller:
    def __init__(self, drone: Drone, joystick: Joystick, keyboard: Keyboard):
        self.drone = drone
        self.joystick = joystick
        self.keyboard = keyboard

        self.keyboard.set_on_event(self.handle_keyboard_event)
        self.joystick.set_on_button_event(self.handle_joystick_button_event)
        self.joystick.set_on_disconnect_event(self.handle_joystick_disconnect_event)

        self.terminate = False

        self.offset_yaw = 0
        self.step = 0.05

        self.P = 10
        self.P1 = 10
        self.P2 = 40


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

    def handle_joystick_button_event(self, button: JoystickButton, active: bool):
        if button == JoystickButton.Trigger and active and self.drone.mode != FlightMode.Safe:
            self.drone.change_mode(FlightMode.Panic)

    def handle_joystick_disconnect_event(self):
        if self.drone.mode != FlightMode.Safe:
            print("Joystick disconnected...")
            self.drone.change_mode(FlightMode.Panic)

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
                self.drone.change_mode(FlightMode.Calibrate)
                pass
            if event.key == pygame.K_4:  # yaw rate
                self.drone.change_mode(FlightMode.Yaw)

            if event.key == pygame.K_u:
                self.P = self.P + 0.1
                self.drone.set_params(id=0, value=self.P)

            if event.key == pygame.K_j:
                self.P = self.P - 0.1
                self.drone.set_params(id=0, value=self.P)

            if event.key == pygame.K_i:
                self.P1 = self.P1 + 0.1
                self.drone.set_params(id=1, value=self.P1)

            if event.key == pygame.K_k:
                self.P1 = self.P1 - 0.1
                self.drone.set_params(id=1, value=self.P1)

            if event.key == pygame.K_o:
                self.P2 = self.P2 + 0.1
                self.drone.set_params(id=2, value=self.P2)

            if event.key == pygame.K_l:
                self.P2 = self.P2 - 0.1
                self.drone.set_params(id=2, value=self.P2)

    def update_inputs(self):
        self.input_roll = self.joystick.get_axis(JoystickAxis.Roll)
        self.input_pitch = self.joystick.get_axis(JoystickAxis.Pitch)
        self.input_yaw = self.joystick.get_axis(JoystickAxis.Yaw)
        self.input_throttle = self.joystick.get_axis(JoystickAxis.Throttle)

    def at_deadpoint(self, x):
        if (x > 120) and (x < 134):
            return True
        return False

    def input_safe(self):
        self.update_inputs()
        return (self.input_throttle == 0) and self.at_deadpoint(self.input_yaw) and self.at_deadpoint(self.input_pitch) and self.at_deadpoint(self.input_roll)

    def map(self, x):
        return int((x + 1) * 127)

    def thread_function(self):
        while not self.terminate:
            if self.joystick.available():
                if self.drone.mode == FlightMode.Manual:
                    self.update_inputs()


                    self.drone.set_control(roll=self.input_roll, pitch=self.input_pitch, yaw=self.input_yaw,throttle= self.input_throttle)

            time.sleep(0.01)

    def stop(self):
        self.terminate = True

    def join(self):
        self.thread.join()
