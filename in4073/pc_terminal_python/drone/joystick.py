from drone.pygame import pygame
from enum import Enum


class JoystickAxis(Enum):
    Roll = 0
    Pitch = 1
    Yaw = 2
    Throttle = 3


class JoystickButton(Enum):
    Trigger = 0
    Thumb = 1
    B3 = 2
    B4 = 3
    B5 = 4
    B6 = 5
    B7 = 6
    B8 = 7
    B9 = 8
    B10 = 9
    B11 = 10
    B12 = 11


def map_to(value, mapping: tuple, limit_input=True):
    x_s, x_e, y_s, y_e = mapping

    # if true then the input will be limited to the x-interval
    # this is used to create a deadzone
    if limit_input:
        if x_s < x_e:  # normal x-axis
            if value < x_s:
                value = x_s
            elif value > x_e:
                value = x_e
        else:  # inverted x-axis
            if value > x_s:
                value = x_s
            elif value < x_e:
                value = x_e

    return (((value - x_s) / (x_e - x_s)) * (y_e - y_s)) + y_s


class Joystick:
    def __init__(self, run_separately=False):
        self.__events = [
            pygame.JOYAXISMOTION,
            pygame.JOYBALLMOTION,
            pygame.JOYBUTTONDOWN,
            pygame.JOYBUTTONUP,
            pygame.JOYDEVICEADDED,
            pygame.JOYDEVICEREMOVED,
            pygame.JOYHATMOTION
        ]

        self.get_events = run_separately
        if run_separately:
            pygame.init()

        self.__joystick = None
        self.__map_axis = {}
        self.__round_axis = {}
        self.__available = False

        self.__init_joystick()

        self.__on_event = None

    def __init_joystick(self):
        pygame.joystick.init()

        count = pygame.joystick.get_count()
        if count > 0:
            self.__joystick = pygame.joystick.Joystick(0)
            self.__joystick.init()

            self.__raw_axis = {}
            self.__parsed_axis = {}
            self.__num_axis = self.__joystick.get_numaxes()

            self.__buttons = {}
            self.__num_buttons = self.__joystick.get_numbuttons()

            self.__available = True
            print("{} joystick with {} axis and {} buttons found!".format(self.__joystick.get_name(), self.__num_axis, self.__num_buttons))

        else:
            self.__joystick = None

    def available(self):
        return self.__available

    def events(self):
        return self.__events

    def __parse_axis(self, axis, raw):
        parsed = raw
        if axis in self.__map_axis:
            parsed = map_to(parsed, self.__map_axis[axis])

        if axis in self.__round_axis:
            parsed = round(parsed)

        return parsed

    def pass_event(self, event):
        if self.__on_event:
            self.__on_event(event)

    def set_on_event(self, handler):
        self.__on_event = handler

    def update(self):
        if self.available:
            if self.get_events:
                pygame.event.get()

            for i in range(self.__num_axis):
                self.__raw_axis[i] = self.__joystick.get_axis(i)
                self.__parsed_axis[i] = self.__parse_axis(i, self.__raw_axis[i])

            for i in range(self.__num_buttons):
                self.__buttons[i] = (self.__joystick.get_button(i) == 1)

    def set_axis_map(self, axis: JoystickAxis, mapping: tuple, do_round):
        self.__map_axis[axis.value] = mapping
        self.__round_axis[axis.value] = do_round

    def get_axis(self, axis: JoystickAxis):
        if axis.value in self.__parsed_axis:
            return self.__parsed_axis[axis.value]
