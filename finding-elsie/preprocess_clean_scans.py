import os
import time
import math
import tqdm
import numpy as np
import matplotlib.pyplot as plt
from sklearn.neighbors import NearestNeighbors

from utils import *

def clean_scans(min_neighbors=1, eps=0.01, scan_dir="data/raw_scans", export_file="data/lab_points.txt", draw=True):
    """
    Clean the LiDAR scans from noise and only keep the points with high certainty 
    """
    files = [os.path.join(scan_dir, f) for f in os.listdir(scan_dir)]
    num_files = len(files)

    point_sets = [file_to_numpy(f) for f in files]

    # We build a KDTree for each sample separately
    kd_trees = []
    for i in range(num_files):
        kd_trees.append(NearestNeighbors(n_neighbors=1).fit(point_sets[i]))
    
    # Keep only points "sure" points
    sure_points = []
    for i in tqdm.tqdm(range(num_files)):
        for j in range(point_sets[i].shape[0]):
            num_close_points = 0
            p = point_sets[i][j, :]
            for k in range(num_files):
                if k == i:
                    continue
                d, _ = kd_trees[k].kneighbors(np.array([p]))
                if d[0][0] < eps:
                    num_close_points += 1
            
            if num_close_points >= min_neighbors:
                flag = False
                for j, q in enumerate(sure_points):
                    d_ = ((p[0] - q[0]) ** 2 + (p[1] - q[1]) ** 2) ** 0.5
                    if d_ < eps:
                        flag = True
                        sure_points = sure_points[:j] + [0.5 * (p + q)] + sure_points[j+1:]
                        break
                if not flag:
                    sure_points.append(p)
    
    # Export the clean data
    points = np.zeros((len(sure_points), 2))
    for i in range(len(sure_points)):
        points[i] = sure_points[i]
    numpy_to_file(points, export_file)

    if draw:
        plt.scatter(points[:, 0], points[:, 1], color='black', s=3)
        plt.show()
        plt.waitforbuttonpress()


if __name__ == "__main__":
    clean_scans(min_neighbors=2, eps=0.05)
    
