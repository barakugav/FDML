import sys
import math

import cv2
import tqdm
import mcubes
import numpy as np
import shapely.geometry

IMG_SIZE = 1024
IMG_MARGIN = 0.5
EPS = 0 #0.08


def implicit_manifold_to_polygon_soup(f, crit, N, minn, maxx):
    u = np.zeros((N,N,N))
    for i in tqdm.tqdm(range(N)):
        for j in range(N):
            x = minn + (maxx - minn) * i / (N-1)
            y = minn + (maxx - minn) * j / (N-1)
            if not crit(x, y):
                u[i,j,:] = 1
                continue
            for k in range(N):    
                theta = 0 + 2 * math.pi * k / (N-1)
                u[i,j,k] = f(x,y,theta)

    vertices, triangles = mcubes.marching_cubes(u, EPS)
    return vertices, triangles


if __name__ == "__main__":
    # Read the csv-ish file
    N = 300
    u = np.zeros((N,N,N))
    with open('build/tmp.csv', 'r') as fp:
        for line in fp.readlines():
            x, y, t, f = line.split(',')
            u[int(x),int(y),int(t)] = float(f)
    
    vertices, triangles = mcubes.marching_cubes(u, EPS)
    mcubes.export_obj(vertices, triangles, 'out/tmp.obj')