from drone.pygame import pygame
from enum import Enum

    #authored by Nathan
class JoystickAxis(Enum):
    Roll = 0
    Pitch = 1
    Yaw = 2
    Throttle = 3

    #authored by Nathan
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

    #authored by Nathan
def map_to(value, mapping: tuple, limit_input=True):
    x_s, x_e, y_s, y_e = mapping

    # if true then the input will be limited to the x-interval
    # this is used to create a deadzone
    if limit_input:
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

        self.__on_button_event = None
        self.__on_disconnect_event = None
        self.__on_connect_event = None
    #authored by Nathan
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
    #authored by Nathan
    def available(self):
        return self.__available
    #authored by Nathan
    def events(self):
        return self.__events
    #authored by Nathan
    def __parse_axis(self, axis, raw):
        parsed = raw

        if axis in self.__map_axis:
            parsed = self.__map_axis[axis](parsed)

        if axis in self.__round_axis:
            parsed = round(parsed)

        return parsed
    #authored by Nathan
    def __handle_event(self, event):
        if event.type == pygame.JOYBUTTONDOWN:
            if self.__on_button_event:
                self.__on_button_event(JoystickButton(event.button), True)
        elif event.type == pygame.JOYBUTTONUP:
            if self.__on_button_event:
                self.__on_button_event(JoystickButton(event.button), False)
        elif event.type == pygame.JOYDEVICEREMOVED:
            self.__available = False

            if self.__on_disconnect_event:
                self.__on_disconnect_event()

        elif event.type == pygame.JOYDEVICEADDED:
            self.__init_joystick()

            if self.__on_connect_event:
                self.__on_connect_event()
    #authored by Nathan
    def pass_event(self, event):
        self.__handle_event(event)
    #authored by Nathan
    def set_on_button_event(self, handler):
        self.__on_button_event = handler
    #authored by Nathan
    def set_on_disconnect_event(self, handler):
        self.__on_disconnect_event = handler
    #authored by Nathan
    def set_on_connect_event(self, handler):
        self.__on_connect_event = handler
    #authored by Nathan
    def update(self):
        if self.__available:
            if self.get_events:
                for event in pygame.event.get():
                    self.__handle_event(event)

            for i in range(self.__num_axis):
                self.__raw_axis[i] = 0#self.__joystick.get_axis(i)
                self.__parsed_axis[i] = self.__parse_axis(i, self.__raw_axis[i])

            for i in range(self.__num_buttons):
                self.__buttons[i] = (self.__joystick.get_button(i) == 1)
    #authored by Nathan
    def set_axis_map(self, axis: JoystickAxis, mapping, do_round):
        # if mapping is a tuple then it will assume a linear mapping
        self.__map_axis[axis.value] = (lambda x: map_to(x, mapping)) if (type(mapping) is tuple) else mapping
        self.__round_axis[axis.value] = do_round
    #authored by Nathan
    def get_axis(self, axis: JoystickAxis):
        if 0:
            if axis.value in self.__parsed_axis:
                return self.__parsed_axis[axis.value]
    #authored by Nathan
    def get_axis_raw(self, axis: JoystickAxis):
        if axis.value in self.__parsed_axis:
            return self.__raw_axis[axis.value]
