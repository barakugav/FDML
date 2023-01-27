import time
import math
import numpy as np

import robomaster
from robomaster import robot, led

from utils import *

dist_buffer_front = []
dist_buffer_back = []

def cache_distance(dist):
    """
    Distance measurement callback. Caches both sensors into the global buffers
    """
    t = time.time()
    dist_buffer_front.append((dist[0], t, True))
    dist_buffer_back.append((dist[1], t, False))


def get_lidar(ep_chassis, transform=lambda x, y: (x,y)):
    """
    Get the entire 360 degree scan from the current position
    Transform input is given from outside to map the relative samples to global space
    """
    points = []

    # Spin in place (and measure distances in the meantime)
    t0 = time.time()
    ep_chassis.move(x=0, y=0, z=360, z_speed=30).wait_for_completed()
    t1 = time.time()

    for d, t, is_front in dist_buffer_front + dist_buffer_back:
        if t < t0 or t > t1 or d > 10 * 1000:
            continue
        theta = (t - t0) / (t1 - t0) * 360
        if not is_front:
            theta += 180
        theta = math.radians(theta)
        d = d / 1000
        x = d * math.cos(theta)
        y = d * math.sin(theta)
        points.append(transform(x, y))
    
    np_points = np.zeros((len(points), 2))
    for i, (x, y) in enumerate(points):
        np_points[i,0] = x
        np_points[i,1] = y
    return np_points

def scan_room(move_dist=0.2, num_steps=14, sample_freq=50, file_prefix="data/raw_scans/"):
    """
    Scan the room (with 360) for given number of steps across the room.
    Output each position along the way to "`file_prefix`+points_{i}.txt"
    """
    # Connect to robot
    ep_robot = robot.Robot()
    ep_robot.initialize(conn_type='ap')

    # Fetch modules
    ep_chassis = ep_robot.chassis
    ep_led = ep_robot.led
    ep_sensor = ep_robot.sensor
    elsie_lights(ep_led)

    # Start sampling
    ep_sensor.sub_distance(sample_freq, callback=cache_distance)

    point_sets = []
    center_x = [0]
    center_y = [0]
    point_sets.append(get_lidar(ep_chassis))
    numpy_to_file(point_sets[-1], file_prefix + "points_{}.txt".format(num_steps))
    for i in range(num_steps):
        # Move forward
        center_x.append(center_x[-1] + move_dist)
        center_y.append(center_y[-1])
        ep_chassis.move(x=move_dist, y=0, z=0, xy_speed=0.7).wait_for_completed()
        time.sleep(1)

        # Get LiDAR and export
        point_sets.append(get_lidar(ep_chassis, lambda x, y: (x+center_x[-1], y)))
        numpy_to_file(point_sets[-1], file_prefix + "points_{}.txt".format(i))
    
    # Thanks, Elsie!
    ep_robot.close()

if __name__ == "__main__":
    scan_room(move_dist=0.4, num_steps=6)