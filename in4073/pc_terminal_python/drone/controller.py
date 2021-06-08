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
        self.offset_pitch = 0
        self.offset_roll = 0
        self.step = 1

        self.yaw = 0
        self.pitch = 0
        self.roll = 0
        self.input_throttle = 0
        self.delta_throttle = 0

        self.P = 18
        self.P1 = 30
        self.P2 = 80

        #gets multiplied by 10 so this adds or sub 50rpm
        self.H = 5


        # start the thread loop now
        self.thread = threading.Thread(target=self.thread_function)
        self.thread.start()

    def inc_check_limits(self, value):
        if (value + self.step) < 127:
            value = value + self.step
        return value

    def dec_check_limits(self, value):
        if (value - self.step) > -127:
            value = value - self.step
        return value

    def handle_joystick_button_event(self, button: JoystickButton, active: bool):
        if button == JoystickButton.Trigger and active and self.drone.mode != FlightMode.Safe:
            self.drone.change_mode(FlightMode.Panic)
        if button == JoystickButton.Thumb:
            if self.drone.mode == FlightMode.Safe and self.input_safe():
                self.drone.change_mode(FlightMode.Manual)
            else:
                print('NOT SAFE')
        if button == JoystickButton.B3:
            self.drone.change_mode(FlightMode.Calibrate)
        if button == JoystickButton.B4:  # yaw rate
            if self.drone.mode == FlightMode.Safe and self.input_safe():
                self.drone.change_mode(FlightMode.Yaw)
            else:
                print("NOT SAFE")
        if button == JoystickButton.B5:  # fullcontrol
            if self.drone.mode == FlightMode.Safe and self.input_safe():
                self.drone.change_mode(FlightMode.Full)
            else:
                print("NOT SAFE")

        if button == JoystickButton.B7:
            if self.drone.mode == FlightMode.Full or self.drone.mode == FlightMode.Raw:
                self.drone.change_mode(FlightMode.HoldHeight)



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
            if event.key == pygame.K_LEFT:
                self.offset_roll = self.dec_check_limits(self.offset_roll)
            if event.key == pygame.K_RIGHT:
                self.offset_roll = self.inc_check_limits(self.offset_roll)
            if event.key == pygame.K_UP:
                self.offset_pitch = self.inc_check_limits(self.offset_pitch)
            if event.key == pygame.K_DOWN:
                self.offset_pitch = self.dec_check_limits(self.offset_pitch)

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

            if event.key == pygame.K_4:  # yaw rate
                if self.drone.mode == FlightMode.Safe and self.input_safe():
                    self.drone.change_mode(FlightMode.Yaw)
                else:
                    print("NOT SAFE")

            if event.key == pygame.K_5:  # fullcontrol
                if self.drone.mode == FlightMode.Safe and self.input_safe():
                    self.drone.change_mode(FlightMode.Full)
                else:
                    print("NOT SAFE")

            if event.key == pygame.K_7:
                if self.drone.mode == FlightMode.Full or self.drone.mode == FlightMode.Raw:
                    self.drone.change_mode(FlightMode.HoldHeight)

            if event.key == pygame.K_u:
                self.P = self.P + 1
                self.drone.set_params(id=0, value=self.P)

            if event.key == pygame.K_j:
                if self.P > 1:
                    self.P = self.P - 1
                    self.drone.set_params(id=0, value=self.P)

            if event.key == pygame.K_i:
                self.P1 = self.P1 + 1
                self.drone.set_params(id=1, value=self.P1)

            if event.key == pygame.K_k:
                if self.P1 > 1:
                    self.P1 = self.P1 - 1
                    self.drone.set_params(id=1, value=self.P1)

            if event.key == pygame.K_o:
                self.P2 = self.P2 + 1
                self.drone.set_params(id=2, value=self.P2)

            if event.key == pygame.K_l:
                if self.P2 > 1:
                    self.P2 = self.P2 - 1
                    self.drone.set_params(id=2, value=self.P2)

            if event.key == pygame.K_y:
                self.H = self.H + 1
                self.drone.set_params(id=3, value=self.H)

            if event.key == pygame.K_h:
                if self.H > 1:
                    self.H = self.H - 1
                    self.drone.set_params(id=3, value=self.H)

    def update_inputs(self):
        self.input_roll = self.joystick.get_axis(JoystickAxis.Roll)
        self.input_pitch = self.joystick.get_axis(JoystickAxis.Pitch)
        self.input_yaw = self.joystick.get_axis(JoystickAxis.Yaw)
        old_throttle = self.input_throttle
        self.input_throttle = self.joystick.get_axis(JoystickAxis.Throttle)
        if self.drone.mode == FlightMode.Full or self.drone.mode == FlightMode.HoldHeight:
            self.delta_throttle = self.input_throttle - old_throttle
            if abs(self.delta_throttle) > 1 and self.drone.mode == FlightMode.HoldHeight:
                print('HoldHeight + Throttle change')
                self.drone.change_mode(FlightMode.Full)
                self.delta_throttle = 0


    def at_deadpoint(self, x):
        if (x > 120) and (x < 134):
            return True
        return False

    def input_safe(self):
        self.update_inputs()
        return (self.input_throttle == 0) and self.at_deadpoint(self.input_yaw) and self.at_deadpoint(self.input_pitch) and self.at_deadpoint(self.input_roll)

    def map(self, x):
        return int((x + 1) * 127)

    def limit(self, x):
        if x > 255:
            x = 255
        if x < 0:
            x = 0
        return x

    def thread_function(self):
        while not self.terminate:
            if self.joystick.available():
                if self.drone.mode in [FlightMode.Manual, FlightMode.Yaw, FlightMode.Full, FlightMode.Raw, FlightMode.HoldHeight]:
                    self.update_inputs()
                    self.yaw = self.limit(self.input_yaw + self.offset_yaw)
                    self.pitch = self.limit(self.input_pitch + self.offset_pitch)
                    self.roll = self.limit(self.input_roll + self.offset_roll)
                    self.drone.set_control(roll=self.roll, pitch=self.pitch, yaw=self.yaw, throttle=self.input_throttle)

            time.sleep(0.0125)

    def stop(self):
        self.terminate = True

    def join(self):
        self.thread.join()
