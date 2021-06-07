from drone.pygame import pygame

from drone.drone import Drone


class GUI:
    def __init__(self, size, drone: Drone):
        self.drone = drone

        pygame.init()
        pygame.font.init()
        self.screen_size = size
        self.screen = pygame.display.set_mode(size)

        self.update_title()

    def update_title(self):
        status = self.drone.mode.name if self.drone.mode else "Disconnected"
        pygame.display.set_caption("Drone ({})".format(status))

    def draw(self):
        width, height = self.screen_size
        rect = pygame.Rect((width / 2) - 25, (height / 2) - 25, 50, 50)

        (phi, theta, psi) = self.drone.get_angles()
        (r0, r1 ,r2 ,r3) = self.drone.get_rpm()

        white = (255, 255, 255)
        green = (0, 255, 0)
        blue = (0, 0, 128)
        black = (0,0,0)

        angle_str = 'Phi: ' + str(phi) + ' Theta: '+str(theta) + ' Psi:'+str(psi)
        rpm_str = 'Rotor RPM: ' + str(r0) + ' ' + str(r1) + ' ' + str(r2) + ' ' + str(r3)

        font = pygame.font.Font('freesansbold.ttf', 32)
        text_angles = font.render(angle_str, True, black, white)
        text_rpms = font.render(rpm_str, True, black, white)
        # textRect = text.get_rect()
        # textRect.center = (width // 8, height // 8)


        # rotated = pygame.transform.rotate(self.screen, (phi + 32767) / 65535)

        self.screen.fill((white))
        pygame.draw.rect(self.screen, (255, 0, 0), rect)
        self.screen.blit(text_angles, (30, 40))
        self.screen.blit(text_rpms, (30, 80))
        pygame.display.flip()



        self.update_title()
