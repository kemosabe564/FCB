import os
os.environ['PYGAME_HIDE_SUPPORT_PROMPT'] = "hide"
import pygame
import threading


class GUI:
    def __init__(self):
        self.terminate = False

        self.thread = threading.Thread(target=self.gui_thread)
        self.thread.start()

    def gui_thread(self):
        while not self.terminate:
            # gui code
            pass

    def stop(self):
        self.terminate = True

    def join(self):
        self.thread.join()
