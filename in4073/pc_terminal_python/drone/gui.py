import os
os.environ['PYGAME_HIDE_SUPPORT_PROMPT'] = "hide"
import pygame
from typing import Callable
from queue import Queue

from drone.drone import Drone


class GUI:
    def __init__(self, size, drone: Drone):
        self.terminate = False

        pygame.init()
        self.screen_size = size
        self.screen = pygame.display.set_mode(size)

        self.on_quit = None
        self.on_event = None

    def set_on_quit(self, handler: Callable):
        self.on_quit = handler

    def set_on_event(self, handler: Callable):
        self.on_event = handler

    def main_loop(self):
        while not self.terminate:
            self.__event_loop()
            self.__draw_loop()

        pygame.quit()

    def __event_loop(self):
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                self.__quit()
            else:
                if self.on_event:
                    self.on_event(event)

    def __draw_loop(self):
        width, height = self.screen_size
        rect = pygame.Rect(width / 2, height / 2, 50, 50)

        self.screen.fill((255, 255, 255))
        pygame.draw.rect(self.screen, (255, 0, 0), rect)
        pygame.display.flip()

    def __quit(self):
        if self.on_quit is not None:
            self.on_quit()

        self.stop()

    def stop(self):
        self.terminate = True

    def join(self):
        self.thread.join()
