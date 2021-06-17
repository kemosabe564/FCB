import numpy as np
import math


class Wireframe:
    def __init__(self):
        self.shape = np.zeros((0, 4))
        self.edges = []
        self.angles = (0.5, 0, 0)

    def addNodes(self, node_array):
        ones_column = np.ones((len(node_array), 1))
        ones_added = np.hstack((node_array, ones_column))
        self.shape = np.vstack((self.shape, ones_added))

    def addEdges(self, edgeList):
        self.edges += edgeList

    def get_points(self):
        center = self.findCentre()
        (x, y, z) = self.angles

        nodes = self.rotateX(center, self.shape, x)
        nodes = self.rotateY(center, nodes, y)
        nodes = self.rotateZ(center, nodes, z)

        return nodes

    def findCentre(self):
        num_nodes = len(self.shape)
        meanX = sum([node[0] for node in self.shape]) / num_nodes
        meanY = sum([node[1] for node in self.shape]) / num_nodes
        meanZ = sum([node[2] for node in self.shape]) / num_nodes

        return meanX, meanY, meanZ

    def rotateX(self, point, nodes, radians):
        (cx, cy, cz) = point

        copy = np.vstack(nodes)
        for node in copy:
            y = node[1] - cy
            z = node[2] - cz
            d = math.hypot(y, z)
            theta = math.atan2(y, z) + radians
            node[2] = cz + d * math.cos(theta)
            node[1] = cy + d * math.sin(theta)

        return copy

    def rotateY(self, point, nodes, radians):
        (cx, cy, cz) = point

        copy = np.vstack(nodes)
        for node in copy:
            x = node[0] - cx
            z = node[2] - cz
            d = math.hypot(x, z)
            theta = math.atan2(x, z) + radians
            node[2] = cz + d * math.cos(theta)
            node[0] = cx + d * math.sin(theta)

        return copy

    def rotateZ(self, point, nodes, radians):
        (cx, cy, cz) = point

        copy = np.vstack(nodes)
        for node in copy:
            x = node[0] - cx
            y = node[1] - cy
            d = math.hypot(y, x)
            theta = math.atan2(y, x) + radians
            node[0] = cx + d * math.cos(theta)
            node[1] = cy + d * math.sin(theta)

        return copy

    def set_angles(self, angles):
        self.angles = angles

