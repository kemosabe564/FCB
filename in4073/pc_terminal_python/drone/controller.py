import pygame
import threading

from drone.drone import Drone

# Joystick and keyboard stuff


class Controller:
    def __init__(self, drone: Drone):
        self.drone = drone

        self.terminate = False
        self.thread = threading.Thread(target=self.thread_function)
        self.thread.start()

    def thread_function(self):
        while not self.terminate:
            # controller reading and parsing loop
            # if KEYEVENT = '1'
            #     drone.change_mode(1)
            pass


    def stop(self):
        self.terminate = True

    def join(self):
        self.thread.join()
