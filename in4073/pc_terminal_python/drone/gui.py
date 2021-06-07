from drone.pygame import pygame

from drone.drone import Drone, FlightMode
from drone.controller import Controller



class GUI:
    def __init__(self, size, drone: Drone , controller: Controller):
        self.drone = drone
        self.controller = controller

        pygame.init()
        pygame.font.init()
        self.screen_size = size
        self.screen = pygame.display.set_mode(size)

        self.update_title()

    def update_title(self):
        status = self.drone.mode.name if self.drone.mode else "Disconnected"
        pygame.display.set_caption("Drone ({})".format(status))

    def get_torques(self):
        (r0, r1, r2, r3) = self.drone.get_rpm()
        yaw_torque = r1**2 + r3**2 - r0**2 - r2**2
        roll_torque = r3**2 - r1**2
        pitch_torque = r0**2 - r2**2
        torques_str = 'Generated - Yaw: '+str(int(yaw_torque/1000))+' Roll: '+str(int(roll_torque/1000)) + ' Pitch: ' + str(int(pitch_torque/1000))
        return torques_str


    def draw(self):
        width, height = self.screen_size
        rect = pygame.Rect((width / 2) - 25, (height / 2) - 25, 50, 50)

        (phi, theta, psi) = self.drone.get_angles()
        (r0, r1, r2, r3) = self.drone.get_rpm()

        white = (255, 255, 255)
        green = (0, 100, 0)
        blue = (0, 0, 128)
        red = (255, 0, 0)
        black = (0,0,0)


        font = pygame.font.Font('freesansbold.ttf', 32)
        font2 = pygame.font.Font('freesansbold.ttf', 16)

        angle_str = 'Phi: ' + str(phi) + ' Theta: '+str(theta) + ' Psi:'+str(psi)
        rpm_str = 'Rotor RPM: ' + str(r0) + ' ' + str(r1) + ' ' + str(r2) + ' ' + str(r3)
        p_str = 'P: ' + str(self.controller.P) + ' P1: ' + str(self.controller.P1) + ' P2: ' + str(self.controller.P2)
        if self.controller.input_safe():
            safe_str = 'Inputs safe'
            text_safe = font2.render(safe_str, True, green, white)
        else:
            safe_str = 'INPUTS NOT SAFE'
            text_safe = font2.render(safe_str, True, black, red)

        text_angles = font.render(angle_str, True, black, white)
        text_rpms = font.render(rpm_str, True, black, white)
        text_torques = font.render(self.get_torques(), True, blue, white)
        text_p = font2.render(p_str, True, black, white)

        # textRect = text.get_rect()
        # textRect.center = (width // 8, height // 8)

        # rotated = pygame.transform.rotate(self.screen, (phi + 32767) / 65535)

        self.screen.fill((white))
        pygame.draw.rect(self.screen, (255, 0, 0), rect)
        self.screen.blit(font.render('Phi: '+str(phi), True, black, white), (30, 40))
        self.screen.blit(font.render('Theta: '+str(theta), True, black, white), (290, 40))
        self.screen.blit(font.render('Psi: '+str(psi), True, black, white), (550, 40))
        self.screen.blit(text_rpms, (30, 80))
        self.screen.blit(text_torques, (30, 120))
        self.screen.blit(text_p, (30, height - 60))
        if self.drone.mode == FlightMode.Safe:
            self.screen.blit(text_safe, (30, height - 90))

        pygame.display.flip()



        self.update_title()
