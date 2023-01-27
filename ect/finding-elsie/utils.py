import numpy as np
from robomaster import led

def elsie_lights(ep_led):
    """
    Set the beautiful colors of Elsie :)
    """
    ep_led.set_led(comp=led.COMP_BOTTOM_BACK, r=147, g=204, b=234, effect=led.EFFECT_ON) # Back
    ep_led.set_led(comp=led.COMP_BOTTOM_FRONT, r=255, g=0, b=193, effect=led.EFFECT_ON) # Right
    ep_led.set_led(comp=led.COMP_BOTTOM_LEFT, r=255, g=0, b=193, effect=led.EFFECT_ON) # Left
    ep_led.set_led(comp=led.COMP_BOTTOM_RIGHT, r=255, g=255, b=255, effect=led.EFFECT_ON) # Front

def numpy_to_file(arr, filename):
    """
    Dump numpy array of 2d points to a *.txt file
    """
    with open(filename, "w") as fp:
        for i in range(arr.shape[0]):
            fp.write("{} {}\n".format(arr[i, 0], arr[i, 1]))

def file_to_numpy(filename):
    """
    Load numpy dump (*.txt file) from file to an array of 2d points
    """
    points = []
    with open(filename, "r") as fp:
        for line in fp.readlines():
            tmp = line.split()
            x = float(tmp[0])
            y = float(tmp[1])
            points.append((x, y))
    np_points = np.zeros((len(points), 2))
    for i, (x, y) in enumerate(points):
        np_points[i][0] = x
        np_points[i][1] = y
    return np_points

def load_polygon(filename):
    polygon = []
    with open(filename, 'r') as fp:
        for line in fp.readlines():
            x, y = line.split()
            polygon.append((float(x), float(y)))
    return polygon

def obj_file_to_numpy(filename):
    """
    Read vertices of an OBJ file as a point cloud to a numpy array
    """
    point_cloud = []
    with open(filename, 'r') as fp:
        for line in fp.readlines():
            if line.startswith('v '):
                tmp = line.split()
                x = float(tmp[1])
                y = float(tmp[2])
                z = float(tmp[3])
                point_cloud.append((x,y,z))
    point_cloud = list(set(point_cloud))
    
    arr = np.zeros((len(point_cloud), 3))
    for i in range(len(point_cloud)):
        x, y, z = point_cloud[i]
        arr[i, 0] = x
        arr[i, 1] = y
        arr[i, 2] = z
    
    return arr

