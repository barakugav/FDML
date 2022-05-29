import sys
import math

import cv2
import tqdm
import mcubes
import numpy as np
import shapely.geometry

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


def draw_polygon(polygon):
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
    cv2.imwrite('test.png', img)


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
            # if not crit(x, y):
            #     u[i, j, :] = 1
            #     continue
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


def intersect_manifolds_with_rotate(f_d1, f_d2, alpha):
    def f_new(x, y, theta):
        val1 = f_d1(x, y, theta)
        val2 = f_d2(x, y, theta + alpha)
        s1 = 1 if val1 > 0 else -1
        s2 = 1 if val2 > 0 else -1
        return (abs(val1) + abs(val2))

    return f_new


def intersect_manifolds_with_two_rotates(f_d1, f_d2, f_d3, alpha1, alpha2):
    def f_new(x, y, theta):
        return abs(f_d1(x, y, theta)) + abs(f_d2(x, y, theta + alpha1)) + abs(f_d3(x, y, theta + alpha2))

    return f_new


def intersect_manifolds_with_shift(f_d1, f_d2, r, beta):
    def f_new(x, y, theta):
        x_ = x + r * math.cos(theta + beta)
        y_ = y + r * math.sin(theta + beta)

        f1 = f_d1(x, y, theta)
        f2 = f_d2(x_, y_, theta)
        sgn = 1 if (np.sign(f1) == 1 and np.sign(f2) == 1) else -1
        return abs(f1) + abs(f2)

    return f_new


def load_polygon(filename):
    polygon = []
    with open(filename, 'r') as fp:
        for line in fp.readlines():
            x, y = line.split()
            polygon.append((float(x), float(y)))
    return polygon


if __name__ == "__main__":
    # polygon = [(-1.5,0), (-1.5,3), (0, 2), (1.5,3), (1.5,0)]
    # polygon = [(-1, -1), (-1, 1), (1, 1), (1, -1)]
    # polygon = [(1,1), (3,4), (6,5), (5,3), (6,1)]
    # polygon = [(-1,-1), (1, -1), (1,1), (-0.5, 1), (-0.5, 0.5), (-0.75, 0.5), (-0.75, 0.75), (-0.51, 0.75), (-0.51, 1), (-1, 1)]
    # polygon = [(-1, -1), (1,-1), (1, 1), (0.05, 1), (0.05, 0.75), (0.125,0.75), (0.125, 0.5), (-0.125, 0.5), (-0.125, 0.75), (-0.05, 0.75), (-0.05, 1), (-1, 1)]
    polygon = load_polygon('meshing/resources/polygons/checkpoint.poly')
    minn, maxx = get_polygon_bb(polygon)
    draw_polygon(polygon)
    # sys.exit(-1)
    def crit(x, y): return is_point_inside(polygon, x, y)

    N = 100
    d1 = 1
    d2 = 2
    # r = 0.1
    beta = math.pi

    f_d1 = generate_f_d(polygon, d1)
    f_d2 = generate_f_d(polygon, d2)
    def f_d2_(x, y, theta): return f_d2(x, y, theta + beta)
    # f_isect = intersect_manifolds_with_shift(f_d1, f_d2, r, beta)
    # f_isect = intersect_manifolds_with_rotate(f_d1, f_d2, beta)

    vertices, triangles = implicit_manifold_to_polygon_soup(
        f_d1, crit, N, minn, maxx)
    mcubes.export_obj(vertices, triangles, 'meshing/tmp/d1_.obj')
    # project_polygon_soup_to_xy('d1.png', triangles, vertices, N)

    vertices, triangles = implicit_manifold_to_polygon_soup(f_d2_, crit, N, minn, maxx)
    mcubes.export_obj(vertices, triangles, 'meshing/tmp/d2_.obj')
    # project_polygon_soup_to_xy('d2_.png', triangles, vertices, N)

    # vertices, triangles = implicit_manifold_to_polygon_soup(f_isect, crit, N, minn, maxx)
    # mcubes.export_obj(vertices, triangles, 'isect.obj')
    # project_polygon_soup_to_xy('isect.png', triangles, vertices, N)

    # vertices, triangles = implicit_manifold_to_polygon_soup(f_d1, crit=lambda x, y: is_point_inside(polygon, x, y), N=N)
    # mcubes.export_obj(vertices, triangles, 'd1.obj')
    # project_polygon_soup_to_xy('d1.png', triangles, vertices, N)
