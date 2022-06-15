import math

import cv2
import tqdm
import mcubes
import numpy as np
import shapely.geometry

from utils import *

"""
IDEA: https://github.com/tdhooper/glsl-marching-cubes
"""

IMG_SIZE = 1024
IMG_MARGIN = 0.5
EPS = 0.08

def get_polygon_bb(polygon):
    minn = None
    maxx = None

    for x, y in polygon:
        if minn is None or x < minn:
            minn = x
        if minn is None or -y < minn:
            minn = -y
        if maxx is None or x > maxx:
            maxx = x
        if maxx is None or -y > maxx:
            maxx = -y

    return minn, maxx

def draw_polygon(polygon, filename):
    """
    Render polygon to image
    """
    minn, maxx = get_polygon_bb(polygon)
    minn, maxx = minn - IMG_MARGIN, maxx + IMG_MARGIN

    img = np.zeros((IMG_SIZE, IMG_SIZE))
    scaled_polygon = np.zeros((len(polygon), 2), dtype=np.int32)
    for i, (x, y) in enumerate(polygon):
        x = (x - minn) / (maxx - minn) * IMG_SIZE
        y = (-y - minn) / (maxx - minn) * IMG_SIZE
        scaled_polygon[i, 0] = x
        scaled_polygon[i, 1] = y
    cv2.polylines(img, [scaled_polygon], 1, color=(255,), thickness=3)
    cv2.imwrite(filename, img)

def get_ray_distance_to_wall(polygon, x, y, theta):
    """
    Rotate the polygon by pi/2 - theta (around origin) and put in an arrangement
    Cast a (vertical) ray and return distance to the first wall/corner we intersect
    """
    theta = math.pi / 2 - theta
    rotated_polygon = []
    cos_theta = math.cos(theta)
    sin_theta = math.sin(theta)
    for x_, y_ in polygon:
        x_, y_ = x_ - x, y_ - y
        rotated_polygon.append((
            x_ * cos_theta - y_ * sin_theta,
            x_ * sin_theta + y_ * cos_theta
        ))

    min_d = None
    for i in range(len(rotated_polygon)):
        p = [rotated_polygon[i][0], rotated_polygon[i][1]]
        q = [rotated_polygon[(i+1) % len(rotated_polygon)][0],
             rotated_polygon[(i+1) % len(rotated_polygon)][1]]

        if not (p[0] <= 0 <= q[0]) and not (q[0] <= 0 <= p[0]):
            continue

        y_ = p[1] - p[0] * (q[1] - p[1]) / (q[0] - p[0])

        if y_ < 0:
            continue
        d = y_
        if min_d is None or d < min_d:
            min_d = d

    if min_d is None:
        return -1.0
    return min_d

def is_point_inside(polygon, x, y):
    poly = shapely.geometry.polygon.Polygon(polygon)
    point = shapely.geometry.Point(x, y)
    return poly.contains(point)


def implicit_manifold_to_polygon_soup(f, crit, N, minn, maxx):
    u = np.zeros((N, N, N))
    for i in tqdm.tqdm(range(N)):
        for j in range(N):
            x = minn + (maxx - minn) * i / (N-1)
            y = minn + (maxx - minn) * j / (N-1)
            if not crit(x, y):
                u[i, j, :] = 1
                continue
            for k in range(N):
                theta = 0 + 2 * math.pi * k / (N-1)
                u[i, j, k] = f(x, y, theta)

    vertices, triangles = mcubes.marching_cubes(u, EPS)
    return vertices, triangles

def generate_f_d(polygon, d):
    """
    Get a polygon and d, and return a function that has 0 if a point (x,y,theta) has distance d to the polygon
    """
    def f_d(x, y, theta):
        return get_ray_distance_to_wall(polygon, x, y, theta) - d

    return f_d

def project_polygon_soup_to_xy(filename, triangles, vertices, N):
    img = np.zeros((IMG_SIZE, IMG_SIZE))
    for tri in triangles:
        a = vertices[tri[0]]
        b = vertices[tri[1]]
        c = vertices[tri[2]]

        new_tri = np.array([
            [int(a[0] / (N-1) * IMG_SIZE), IMG_SIZE -
             int(a[1] / (N-1) * IMG_SIZE)],
            [int(b[0] / (N-1) * IMG_SIZE), IMG_SIZE -
             int(b[1] / (N-1) * IMG_SIZE)],
            [int(c[0] / (N-1) * IMG_SIZE), IMG_SIZE -
             int(c[1] / (N-1) * IMG_SIZE)],
        ])
        img = cv2.fillPoly(img, [new_tri], color=(255, 255, 255))
    img = cv2.resize(img, (IMG_SIZE, IMG_SIZE))
    cv2.imwrite(filename, img)

def single_measurement(polygon_filename, output_filename, N, d, alpha, manifold_image="", polygon_image="data/images/lab.png"):
    polygon = load_polygon(polygon_filename)
    minn, maxx = get_polygon_bb(polygon)
    minn -= 1
    maxx += 1
    draw_polygon(polygon, polygon_image)
    crit = lambda x,y: is_point_inside(polygon, x, y)

    f_d = generate_f_d(polygon, d)
    f_d_rot = lambda x, y, theta: f_d(x, y, theta+alpha)

    vertices, triangles = implicit_manifold_to_polygon_soup(f_d_rot, crit, N, minn, maxx)
    mcubes.export_obj(vertices, triangles, output_filename)
    if manifold_image != "":
        project_polygon_soup_to_xy(manifold_image, triangles, vertices, N)


if __name__ == "__main__":
    ds = [0.69, 2.06, 3.45, 1.46]
    alphas = [0, math.pi / 2, math.pi, 3 * math.pi / 2]

    for i in range(4):
        single_measurement(
            polygon_filename="data/lab.poly",
            output_filename="data/manifolds/m_{}.obj".format(i+1),
            N=100,
            d=ds[i],
            alpha=alphas[i],
            manifold_image="data/images/m_{}.png".format(i+1)
        )