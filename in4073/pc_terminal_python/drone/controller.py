import pygame
import threading
import time


from drone.drone import Drone, FlightMode

# Joystick and keyboard stuff

class Controller:
    def __init__(self, drone: Drone):
        self.drone = drone

        self.terminate = False
        self.joystick_isAvailable = False

        #this is done in gui already
        pygame.init()
        # size = [500, 700]
        # self.screen = pygame.display.set_mode(size)
        #pygame.display.set_caption("My Quadruple game")

        # Initialize the joysticks
        pygame.joystick.init()
        self.joystick_count = pygame.joystick.get_count()
        if (self.joystick_count == 1):
            #Assuming there is only one joystick (should handle an exception if not)
            self.joystick = pygame.joystick.Joystick(0)
            self.joystick.init()
            self.joystick_isAvailable = True
            self.axes = self.joystick.get_numaxes() #not really required but anyway

            #tested on the lab joystick
            #axis[0] = roll : left =-1 and right =+1
            #axis[1] = pitch : forward =-1 and backward =+1
            #axis[2] = yaw : cw = 1 and ccw= -1
            #axis[3] = Throttle : zero =1 and full = -1

            self.input_roll = self.joystick.get_axis(0)
            self.input_pitch = self.joystick.get_axis(1)
            self.input_yaw = self.joystick.get_axis(2)
            self.input_throttle = self.joystick.get_axis(3)

        self.offset_yaw = 0

        self.step = 0.05

        #start the thread loop now
        self.thread = threading.Thread(target=self.thread_function)
        self.thread.start()

    def inc_check_limits(self, value):
        if (value + self.step < 1):
            value = value + self.step
        return value

    def dec_check_limits(self, value):
        if (value - self.step > -1):
            value = value - self.step
        return value

    def handle_event(self, event):
        if event.type == pygame.KEYDOWN:
            if event.key == pygame.K_q:  # yaw down
                self.offset_yaw = self.dec_check_limits(self.offset_yaw)
            if event.key == pygame.K_w:  # yaw up
                self.offset_yaw = self.inc_check_limits(self.offset_yaw)
            if event.key == pygame.K_0:  # safe mode
                self.drone.change_mode(FlightMode.Safe)
            if event.key == pygame.K_1:  # panic mode
                self.drone.change_mode(FlightMode.Panic)
            if event.key == pygame.K_2:  # manual mode
                if self.drone.mode == FlightMode.Safe and self.input_safe():
                    self.drone.change_mode(FlightMode.Manual)
                else:
                    print("NOT SAFE")
            if event.key == pygame.K_3:  # calibration
                # self.drone.change_mode(FlightMode.Calibrate)
                pass
            if event.key == pygame.K_4:  # yaw rate
                self.drone.change_mode(FlightMode.Yaw)

    def update_keys(self):
        pass
        # for event in pygame.event.get():
        #     if event.type == pygame.KEYDOWN:
        #         if event.key == pygame.K_q: #yaw down
        #             self.offset_yaw = self.dec_check_limits(self.offset_yaw)
        #         if event.key == pygame.K_w: #yaw up
        #             self.offset_yaw = self.inc_check_limits(self.offset_yaw)
        #         if event.key == pygame.K_0: #safe mode
        #             #TODO:Send cmod command
        #             pass
        #         if event.key == pygame.K_1: #panic mode
        #             #TODO:Send cmod command
        #             pass
        #         if event.key == pygame.K_2: #manual mode
        #             #TODO:Send cmod command
        #             pass
        #         if event.key == pygame.K_3: #calibration
        #             #TODO:Send cmod command
        #             pass
        #         if event.key == pygame.K_4: #yaw rate
        #             #TODO:Send cmod command
        #             pass

    #TODO: This needs to be changed to limit to [-1,+1]
    #TODO: Check what this needs to be mapped to
    def update_inputs(self):
        self.input_roll = self.map(self.joystick.get_axis(0))
        self.input_pitch = self.map(self.joystick.get_axis(1))
        yaw = self.joystick.get_axis(2) + self.offset_yaw
        if yaw >= 1:
            yaw = 1
        elif yaw <= -1:
            yaw = -1
        self.input_yaw = self.map(yaw)
        self.input_throttle = self.map(-self.joystick.get_axis(3))

        # print(self.input_throttle, self.input_roll, self.input_pitch, self.input_yaw)

        if self.drone.mode == FlightMode.Manual:
            self.drone.set_control(yaw=self.input_yaw, pitch=self.input_pitch, roll=self.input_roll, throttle=self.input_throttle)

    def at_deadpoint(self, x):
        if (x > 120) and (x < 134):
            return True
        return False

    def input_safe(self):
        return (self.input_throttle <= 10) and self.at_deadpoint(self.input_yaw) and self.at_deadpoint(self.input_pitch) and self.at_deadpoint(self.input_roll)

    def map(self, x):
        return int((x + 1) * 127)

    def thread_function(self):
        while not self.terminate:
            if self.joystick_isAvailable:
                self.update_keys()
                self.update_inputs()
                #TODO: Implement this sending

                # self.drone.set_control(self.input_yaw, self.input_roll, self.input_pitch, self.input_throttle)

            time.sleep(0.02)
            # controller reading and parsing loop

            # if KEYEVENT = '1'
            #     drone.change_mode(1)
            pass


    def stop(self):
        self.terminate = True

    def join(self):
        self.thread.join()
