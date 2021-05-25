from drone.pygame import pygame

from drone.drone import Drone


class GUI:
    def __init__(self, size, drone: Drone):
        self.drone = drone

        pygame.init()
        self.screen_size = size
        self.screen = pygame.display.set_mode(size)

        self.update_title()

    def update_title(self):
        status = self.drone.mode.name if self.drone.mode else "Disconnected"
        pygame.display.set_caption("Drone ({})".format(status))

    def draw(self):
        width, height = self.screen_size
        rect = pygame.Rect(width / 2, height / 2, 50, 50)

        self.screen.fill((255, 255, 255))
        pygame.draw.rect(self.screen, (255, 0, 0), rect)
        pygame.display.flip()

        self.update_title()
