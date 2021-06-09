from drone.pygame import pygame
from drone.joystick import Joystick, JoystickAxis, JoystickButton
from drone.keyboard import Keyboard


class Eventloop:
    def __init__(self):
        pygame.init()

        self.__joystick = Joystick()
        self.__joystick.set_axis_map(JoystickAxis.Yaw,      (-1, 1, 0, 255), True)
        self.__joystick.set_axis_map(JoystickAxis.Pitch,    (-1, 1, 0, 255), True)
        self.__joystick.set_axis_map(JoystickAxis.Roll,     (-1, 1, 0, 255),  True)
        self.__joystick.set_axis_map(JoystickAxis.Throttle, (0.85, -1, 0, 255), True)  # 0.15 deadzone

        self.__keyboard = Keyboard()

    def joystick(self):
        return self.__joystick

    def keyboard(self):
        return self.__keyboard

    def update(self):
        for event in pygame.event.get():
            if event.type in self.__joystick.events():
                self.__joystick.pass_event(event)
            elif event.type in self.__keyboard.events():
                self.__keyboard.pass_event(event)

        self.__joystick.update()

