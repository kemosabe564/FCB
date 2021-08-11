from drone.pygame import pygame
from drone.joystick import Joystick, JoystickAxis, JoystickButton
from drone.keyboard import Keyboard
import math

    #authored by Nathan
def tangent_map(x):
    value = 235*math.atan(-x + 0.85)

    if value < 0:
        return 0
    if value > 255:
        return 255

    return value


class Eventloop:
    #authored by Nathan
    def __init__(self):
        pygame.init()

        self.__joystick = Joystick()
        if self.__joystick.available:
            self.__joystick.set_axis_map(JoystickAxis.Yaw,      (-1, 1, 0, 255), True)
            self.__joystick.set_axis_map(JoystickAxis.Pitch,    (-1, 1, 0, 255), True)
            self.__joystick.set_axis_map(JoystickAxis.Roll,     (-1, 1, 0, 255),  True)
            # self.__joystick.set_axis_map(JoystickAxis.Throttle, (0.85, -1, 0, 255), True)  # 0.15 deadzone
        else:
            self.__joystick.set_axis_map(JoystickAxis.Yaw,      (0, 0, 0, 0), True)
            self.__joystick.set_axis_map(JoystickAxis.Pitch,    (0, 0, 0, 0), True)
            self.__joystick.set_axis_map(JoystickAxis.Roll,     (0, 0, 0, 0),  True)
        
        self.__joystick.set_axis_map(JoystickAxis.Throttle, tangent_map, True)

        self.__keyboard = Keyboard()
    #authored by Nathan
    def joystick(self):
        return self.__joystick
    #authored by Nathan
    def keyboard(self):
        return self.__keyboard
    #authored by Nathan
    def update(self):
        for event in pygame.event.get():
            if event.type in self.__joystick.events():
                self.__joystick.pass_event(event)
            elif event.type in self.__keyboard.events():
                self.__keyboard.pass_event(event)

        self.__joystick.update()

