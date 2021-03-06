import matplotlib.backends.backend_agg as agg
import matplotlib.pyplot as plt
from drone.pygame import pygame
from drone.pygame import matplotlib
from drone.pygame import pylab
from drone.drone import Drone, FlightMode
from drone.joystick import Joystick, JoystickAxis, JoystickButton
from drone.controller import Controller
from drone.keyboard import Keyboard
from drone.tripled.wireframe import Wireframe
from drone.tripled.wireframeviewer import WireframeViewer
import numpy as np
import math

#authored by Yiting
def init_fig(Figsize, Dpi):
    fig = pylab.figure(figsize = Figsize, 
                    dpi = Dpi,        
                    )  
    return fig
#authored by Yiting
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
#authored by Yiting
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
    # authored by Nathan
    def __init__(self, size, drone: Drone , controller: Controller):
        self.drone = drone
        self.controller = controller

        pygame.init()
        pygame.font.init()
        self.screen_size = size

        self.screen = pygame.display.set_mode(size)
        #self.screen = pygame.display.set_mode((0, 0), pygame.FULLSCREEN)

        self.wfviewer = WireframeViewer(self.screen)

        cross = Wireframe((0, 0, 255), (30, 30, 230))
        cross_nodes = [(125, 125, 0), (125, 125, 250), (0, 125, 125), (250, 125, 125)]
        cross.addNodes(np.array(cross_nodes))
        cross.addEdges([(0, 1), (2, 3)])

        cube = Wireframe((255, 0, 0), (230, 30, 30))
        cube_nodes = [(x, y, z) for x in (50, 250) for y in (50, 250) for z in (50, 250)]
        cube.addNodes(np.array(cube_nodes))
        cube.addEdges([(n, n + 4) for n in range(0, 4)] + [(n, n + 1) for n in range(0, 8, 2)] + [(n, n + 2) for n in (0, 1, 4, 5)])

        self.wfviewer.add_wireframe('cube', cube)
        self.wfviewer.set_wireframe_position('cube', ((size[0] / 2) - 175, (size[1] / 2) - 175))

        self.wfviewer.add_wireframe('cross', cross)
        self.wfviewer.set_wireframe_position('cross', ((size[0] / 2) - 150, (size[1] / 2) - 150))

        self.update_title()
        self.__init_display()
        #

    # init the text printer and queue for receiving data from joys and snesors
    #authored by Yiting
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
    #authored by Nathan
    def update_title(self):
        status = self.drone.mode.name if self.drone.mode else "Disconnected"
        pygame.display.set_caption("Drone ({})".format(status))
    #authored by Vivian
    def get_torques(self):
        (r0, r1, r2, r3) = self.drone.get_rpm()
        yaw_torque = r1**2 + r3**2 - r0**2 - r2**2
        roll_torque = r3**2 - r1**2
        pitch_torque = r0**2 - r2**2

        self.pitch_data.queue_storing(pitch_torque, 1)
        self.roll_data.queue_storing(roll_torque, 1)
        self.yaw_data.queue_storing(yaw_torque, 1)
        torques_str = 'Generated Torques - Yaw: {}    Roll: {}    Pitch: {}'.format(str(int(yaw_torque/1000)), str(int(roll_torque/1000)), str(int(pitch_torque/1000)))
        return torques_str

    #authored by Vivian
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
        green = (0, 100, 0)
        blue = (0, 0, 128)
        red = (255, 0, 0)
        black = (0, 0, 0)


        font = pygame.font.Font('freesansbold.ttf', 32)
        font2 = pygame.font.Font('freesansbold.ttf', 16)

        angle_str = 'Phi: {: 6d}   Theta: {: 6d}   Psi: {: 6d}'.format(phi, theta, psi)
        rpm_str = 'Rotor RPM {: 4d} {: 4d} {: 4d} {: 4d}'.format(r0, r1, r2, r3)
        p_str = 'P: {: 2d}  P1: {: 2d}  P2: {: 2d}  H: {: 2d}                               Offsets: Yaw: {: 3d}   Roll: {: 3d}   Pitch: {: 3d}'.format(self.controller.P, self.controller.P1, self.controller.P2, self.controller.H, self.controller.offset_yaw,  self.controller.offset_roll, self.controller.offset_pitch)
        inputs_str = 'Inputs: Throttle: {}   Yaw: {}   Roll: {}   Pitch: {} '.format(str(self.controller.input_throttle), str(self.controller.yaw), str(self.controller.roll), str(self.controller.pitch))
        if self.controller.input_safe(self.drone.mode == FlightMode.Safe):
            safe_str = 'Inputs safe'
            text_safe = font2.render(safe_str, True, green, white)
        else:
            safe_str = 'INPUTS NOT SAFE'
            text_safe = font2.render(safe_str, True, black, red)

        if self.controller.battery_check:
            bat_str = 'Battery check enabled. Press z to disable'
            text_bat = font2.render(bat_str,True, green,white)
        else:
            bat_str = 'Battery check disabled'
            text_bat = font2.render(bat_str,True,black,red)
        status = self.drone.mode.name if self.drone.mode else "Disconnected"


        text_angles = font.render(angle_str, True, black, white)
        text_rpm = font.render(rpm_str, True, black, white)
        text_torques = font2.render(self.get_torques(), True, blue, white)
        text_p = font2.render(p_str, True, black, white)
        text_inputs = font2.render(inputs_str, True, (102, 0, 51), white)


        self.screen.fill((white))

        self.screen.blit(font.render('{:^10}'.format(status), True, (204, 0, 0), (255, 225, 225)), (30, height - 150))
        self.screen.blit(text_angles, (30,40))
        self.screen.blit(text_rpm, (30, 80))
        self.screen.blit(text_torques, (30, 120))
        self.screen.blit(text_p, (30, height - 60))
        self.screen.blit(text_bat, (700, height - 60))

        if self.drone.mode in [FlightMode.Full , FlightMode.Yaw , FlightMode.Yaw ,FlightMode.HoldHeight] and not self.controller.isCalibrated:
            self.screen.blit(font2.render('Warning: Not Calibrated', True, red, white),(700,height - 90))

        if self.drone.mode != FlightMode.Safe and self.drone.mode != FlightMode.Panic:
            self.screen.blit(text_inputs, (30, height - 30))

            # draw throttle
            throttle_bg = pygame.Rect(1120, 600, 50, 200)
            pygame.draw.rect(self.screen, (200, 200, 200), throttle_bg)

            #throttle_height = round((self.controller.input_throttle / 255) * 200)
            throttle_height = 150
            throttle = pygame.Rect(1120, 800 - throttle_height, 50, throttle_height)
            pygame.draw.rect(self.screen, (255, 40, 40), throttle)
            # !draw throttle

        if self.drone.mode == FlightMode.Safe:
            self.screen.blit(text_safe, (30, height - 90))

        if self.controller.draw_graphs:
            graph_drawing(self.phi_fig, self.phi_data.data_queue, [-1, 105], [-127, 127], (900, 50), self.screen, 'Phi')
            graph_drawing(self.phi_fig, self.theta_data.data_queue, [-1, 105], [-127, 127], (900, 275), self.screen, 'Theta')
            graph_drawing(self.phi_fig, self.psi_data.data_queue, [-1, 105], [-127, 127], (900, 500), self.screen, 'Psi')


        self.wfviewer.rotate(((phi / 256) * math.pi * 2, (psi / 256) * math.pi * 2, (theta / 256) * math.pi * 2))

        self.wfviewer.display()

        pygame.display.flip()
        self.update_title()
