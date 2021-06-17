from drone.pygame import pygame
import numpy as np

class WireframeViewer:
    """ Displays 3D objects on a Pygame screen """

    def __init__(self, screen):
        self.screen = screen

        self.wireframes = {}
        self.displayNodes = True
        self.displayEdges = True
        self.nodeColour = (255, 0, 0)
        self.edgeColour = (200, 0, 0)
        self.nodeRadius = 4

        self.positions = {}
        self.positions.setdefault((0, 0))

    def set_wireframe_position(self, name, position: tuple):
        self.positions[name] = np.array(position)

    def add_wireframe(self, name, wireframe):
        """ Add a named wireframe object. """

        self.wireframes[name] = wireframe

    def display(self):
        """ Draw the wireframes on the screen. """

        for name, wireframe in self.wireframes.items():
            pos = self.positions[name]
            nodes = wireframe.get_points()

            if self.displayEdges:
                for n1, n2 in wireframe.edges:
                    pygame.draw.aaline(self.screen, self.edgeColour, pos + nodes[n1][:2], pos + nodes[n2][:2], 1)

            if self.displayNodes:
                for node in nodes:
                    pygame.draw.circle(self.screen, self.nodeColour, (pos[0] + int(node[0]), pos[1] + int(node[1])), self.nodeRadius, 0)

    def scale(self, scale):
        """ Scale all wireframes by a given scale, centred on the centre of the screen. """

        centre_x = self.width / 2
        centre_y = self.height / 2

        for wireframe in self.wireframes.itervalues():
            wireframe.scale((centre_x, centre_y), scale)

    def rotate(self, angles):
        """ Rotate all wireframe about their centre, along a given axis by a given angle. """

        for wireframe in self.wireframes.values():
            wireframe.set_angles(angles)
