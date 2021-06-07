import matplotlib.backends.backend_agg as agg
import matplotlib.pyplot as plt

from drone.pygame import pygame

from drone.pygame import matplotlib

from drone.pygame import pylab

from drone.drone import Drone, FlightMode

from drone.joystick import Joystick, JoystickAxis, JoystickButton

from drone.controller import Controller

from drone.keyboard import Keyboard


def init_fig(Figsize, Dpi):
    fig = pylab.figure(figsize = Figsize, 
                    dpi = Dpi,        
                    )  
    return fig

def graph_drawing(fig, data, xlim, ylim, position, screen, title):
    # figzie Inches, dpi dots per inch
    # fig = pylab.figure(figsize = Figsize, 
    #                 dpi = Dpi,        
    #                 )                 
    ax = fig.gca()
    ax.cla()
    ax.plot(data) 
    ax.set_xlim(xlim)
    ax.set_ylim(ylim)
    ax.set_title(title)
    
    canvas = agg.FigureCanvasAgg(fig)
    canvas.draw()
    renderer = canvas.get_renderer()
    raw_data = renderer.tostring_rgb()  

    size = canvas.get_width_height()

    # surf and screen are from pygame
    surf = pygame.image.fromstring(raw_data, size, "RGB")
    screen.blit(surf, position) 

class Display_Data_Queue():
    # rename as Display_Data_Queue
    def __init__(self, N):
        self.data_queue = []
        self.queue_max_length = N
        self.queue_reset()
        
    def queue_reset(self):
        self.data = [0]*self.queue_max_length
        
    def queue_storing(self, newinput, length):
        if length == 1:
            N = self.queue_max_length
            self.data_queue = [newinput] + self.data_queue[0:N-length]
        else:       
            N = self.queue_max_length
            self.data_queue = newinput[0:length] + self.data_queue[0:N-length]



class GUI:
    def __init__(self, size, drone: Drone , controller: Controller):
        self.drone = drone
        self.controller = controller

        pygame.init()
        pygame.font.init()
        self.screen_size = size
        self.screen = pygame.display.set_mode(size)

        self.update_title()
        self.__init_display()
        #

    # init the text printer and queue for receiving data from joys and snesors
    def __init_display(self):
        N = 100
        # for JS
        self.pitch_data   = Display_Data_Queue(N)
        self.roll_data    = Display_Data_Queue(N)
        self.yaw_data     = Display_Data_Queue(N)
        # for Drone
        self.phi_data     = Display_Data_Queue(N)
        self.theta_data   = Display_Data_Queue(N)
        self.psi_data     = Display_Data_Queue(N)
        # for figure
        self.pitch_fig   = init_fig([3, 2], 100)
        self.roll_fig    = init_fig([3, 2], 100)
        self.yaw_fig     = init_fig([3, 2], 100)
        # for Drone
        self.phi_fig     = init_fig([3, 2], 100)
        self.theta_fig   = init_fig([3, 2], 100)
        self.psi_fig     = init_fig([3, 2], 100)
        # 


    def update_title(self):
        status = self.drone.mode.name if self.drone.mode else "Disconnected"
        pygame.display.set_caption("Drone ({})".format(status))

    def get_torques(self):
        (r0, r1, r2, r3) = self.drone.get_rpm()
        yaw_torque = r1**2 + r3**2 - r0**2 - r2**2
        roll_torque = r3**2 - r1**2
        pitch_torque = r0**2 - r2**2

        # self.pitch_data.queue_storing(pygame.joystick.Joystick(0).get_axis(0), 1)
        # self.roll_data.queue_storing(pygame.joystick.Joystick(0).get_axis(1), 1)
        # self.yaw_data.queue_storing(pygame.joystick.Joystick(0).get_axis(2), 1)

        self.pitch_data.queue_storing(pitch_torque, 1)
        self.roll_data.queue_storing(roll_torque, 1)
        self.yaw_data.queue_storing(yaw_torque, 1)
        torques_str = 'Generated - Yaw: '+str(int(yaw_torque/1000))+' Roll: '+str(int(roll_torque/1000)) + ' Pitch: ' + str(int(pitch_torque/1000))
        return torques_str


    def draw(self):
        width, height = self.screen_size
        rect = pygame.Rect((width / 2) - 25, (height / 2) - 25, 50, 50)

        (phi, theta, psi) = self.drone.get_angles()
        phi = int(phi/256)
        theta = int(theta/256)
        psi = int(psi/256)

        self.phi_data.queue_storing(phi, 1)
        self.theta_data.queue_storing(theta, 1)
        self.psi_data.queue_storing(psi, 1)


        (r0, r1, r2, r3) = self.drone.get_rpm()

        white = (255, 255, 255)
        green = (50, 255, 50)
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
            text_safe = font2.render(safe_str, True, red, white)

        text_angles = font.render(angle_str, True, black, white)
        text_rpms = font.render(rpm_str, True, black, white)
        text_torques = font.render(self.get_torques(), True, blue, white)
        text_p = font2.render(p_str, True, black, white)

        # textRect = text.get_rect()
        # textRect.center = (width // 8, height // 8)

        # rotated = pygame.transform.rotate(self.screen, (phi + 32767) / 65535)

        self.screen.fill((white))
        pygame.draw.rect(self.screen, (255, 0, 0), rect)
        self.screen.blit(text_angles, (30, 40))
        self.screen.blit(text_rpms, (30, 80))
        self.screen.blit(text_torques, (30, 120))
        self.screen.blit(text_p, (30, height - 60))
        if self.drone.mode== FlightMode.Safe:
            self.screen.blit(text_safe, (30, height - 90))

        graph_drawing(self.phi_fig, self.phi_data.data_queue, [-1, 105], [-127, 127], (900, 50), self.screen, 'Phi')
        graph_drawing(self.phi_fig, self.theta_data.data_queue, [-1, 105], [-127, 127], (900, 275), self.screen, 'Theta')
        graph_drawing(self.phi_fig, self.psi_data.data_queue, [-1, 105], [-127, 127], (900, 500), self.screen, 'Psi')
        

        # graph_drawing(self.pitch_fig, self.pitch_data.data_queue, [-1, 105], [-1.1, 1.1], (25, 150), self.screen)
        # graph_drawing(self.pitch_fig, self.roll_data.data_queue, [-1, 105], [-1.1, 1.1], (25, 375), self.screen)
        # graph_drawing(self.pitch_fig, self.yaw_data.data_queue, [-1, 105], [-1.1, 1.1], (25, 600), self.screen)



        pygame.display.flip()



        self.update_title()
