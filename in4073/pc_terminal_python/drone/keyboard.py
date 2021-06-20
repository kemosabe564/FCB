from drone.pygame import pygame

class Keyboard:
    def __init__(self):
        self.__events = [
            pygame.KEYUP,
            pygame.KEYDOWN,
        ]

        self.__on_event = None
    #authored by Nathan
    def events(self):
        return self.__events
    #authored by Nathan
    def set_on_event(self, handler):
        self.__on_event = handler
    #authored by Nathan
    def pass_event(self, event):
        if self.__on_event:
            self.__on_event(event)
