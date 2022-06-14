import random
import alphashape
import numpy as np
import matplotlib.pyplot as plt
from descartes import PolygonPatch

from utils import *

def alpha_shape(alpha=0.0, additional_points=[], data_file="data/lab_points.txt", out_file="data/lab.poly", draw=True):
    """
    Use alpha shapes to get the contour of the room
    """
    np_points = file_to_numpy(data_file)
    points = []
    for i in range(np_points.shape[0]):
        points.append((np_points[i, 0], np_points[i, 1]))
    points += additional_points

    alpha_shape = alphashape.alphashape(points, alpha)
    x, y = alpha_shape.exterior.coords.xy
    with open(out_file, "w") as fp:
        for i in range(len(x) - 1): # Last vertex is duplicated
            fp.write("{} {}\n".format(x[i], y[i]))

    if draw:
        fig, ax = plt.subplots()
        ax.scatter(*zip(*points), color='black', s=3)
        ax.add_patch(PolygonPatch(alpha_shape, alpha=0.4))
        plt.show()



if __name__ == "__main__":
    # Add a small line segment to fix ambiguity
    start = (3.41, -1.54)
    end = (3.79, -1.69)
    additional_points = []
    for i in range(101):
        t = i / 100
        p = (
            t * end[0] + (1-t) * start[0] + random.random() * 0.01,
            t * end[1] + (1-t) * start[1] + random.random() * 0.01
        )
        additional_points.append(p)
    alpha_shape(alpha=5.0, additional_points=additional_points, draw=True)