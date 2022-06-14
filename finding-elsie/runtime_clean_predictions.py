import math

import numpy as np
import shapely.geometry
import matplotlib.pyplot as plt
from sklearn.cluster import DBSCAN

from runtime_single_measurement import *
from utils import *

CUTOFF = 5000
N = 100

if __name__ == "__main__":
    filename = "data/manifolds/m3_1234.obj"
    polygon_filename = "data/lab.poly"
    points = obj_file_to_numpy(filename)

    clustering = DBSCAN(eps=3, min_samples=2).fit(points)
    unique, counts = np.unique(clustering.labels_, return_counts=True)
    d = dict(zip(unique, counts))
    good_labels = [label for label in d if d[label] <= CUTOFF]

    predictions = [clustering.components_[label] for label in good_labels]
    # print(predictions)

    # Scale predictions back to real world
    polygon = load_polygon(polygon_filename)
    minn, maxx = get_polygon_bb(polygon)
    minn -= 1
    maxx += 1
    ds = [0.69, 2.06, 3.45, 1.46]
    # ds = [0.48, 0.73, 1.98, 0.73]
    alphas = [0, math.pi / 2, math.pi, 3 * math.pi / 2]
    scaled_predictions = []
    xs = []
    ys = []
    us = []
    vs = []
    for p in predictions:
        x = minn + (p[0] / (N-1) ) * (maxx - minn)
        y = minn + (p[1] / (N-1) ) * (maxx - minn)
        z = (p[2] / (N-1)) * 2 * math.pi

        if not is_point_inside(polygon, x, y):
            continue

        poly = shapely.geometry.polygon.Polygon(polygon)
        point = shapely.geometry.Point(x, y)
        if poly.exterior.distance(point) < 0.2:
            continue

        err = 0
        for i in range(4):
            d = get_ray_distance_to_wall(polygon, x, y, z + alphas[i])
            err += (d - ds[i]) ** 2
        err = math.sqrt(err)
        print(err)

        # if err > 1.5:
        #     continue

        xs.append(x)
        ys.append(y)
        us.append(math.cos(z))
        vs.append(math.sin(z))
        scaled_predictions.append((x, y, z))

    # print(scaled_predictions)
    
    # Draw predictions inside room
    poly_x = [p[0] for p in polygon]
    poly_y = [p[1] for p in polygon]
    poly_x.append(poly_x[0])
    poly_y.append(poly_y[0])
    plt.plot(poly_x, poly_y, color='black')
    plt.quiver(xs, ys, us, vs, color='red')
    plt.show()

    points = points[clustering.core_sample_indices_, :]
    

    # fig = plt.figure()
    # ax = fig.add_subplot(projection='3d')
    # ax.scatter(points[:,0], points[:,1], points[:,2], c=clustering.labels_, s=3)
    # plt.show()
