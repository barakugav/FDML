import time

from runtime_single_measurement import *
from runtime_mia import *

import robomaster
from robomaster import robot, led

dist_buffer_front = []
dist_buffer_back = []

def cache_distance(dist):
    """
    Distance measurement callback. Caches both sensors into the global buffers
    """
    t = time.time()
    dist_buffer_front.append((dist[0], t, True))
    dist_buffer_back.append((dist[1], t, False))

def get_ds():    
    # Connect to robot
    ep_robot = robot.Robot()
    ep_robot.initialize(conn_type='ap')

    # Fetch modules
    ep_chassis = ep_robot.chassis
    ep_led = ep_robot.led
    ep_sensor = ep_robot.sensor
    elsie_lights(ep_led)

    # Start sampling
    ep_sensor.sub_distance(10, callback=cache_distance)

    time.sleep(1)
    t0 = time.time()
    time.sleep(0.1)
    t1 = time.time()

    d1 = 0
    cnt = 0
    for d, t, _ in dist_buffer_front:
        if t < t0 or t > t1:
            continue
        d1 += d
        cnt += 1
    d1 = d1 / cnt / 1000

    d3 = 0
    cnt = 0
    for d, t, _ in dist_buffer_back:
        if t < t0 or t > t1:
            continue
        d3 += d
        cnt += 1
    d3 = d3 / cnt / 1000


    ep_chassis.move(x=0, y=0, z=90, z_speed=30).wait_for_completed()
    time.sleep(1)
    t0 = time.time()
    time.sleep(0.1)
    t1 = time.time()

    d2 = 0
    cnt = 0
    for d, t, _ in dist_buffer_front:
        if t < t0 or t > t1:
            continue
        d2 += d
        cnt += 1
    d2 = d2 / cnt / 1000

    d4 = 0
    cnt = 0
    for d, t, _ in dist_buffer_back:
        if t < t0 or t > t1:
            continue
        d4 += d
        cnt += 1
    d4 = d4 / cnt / 1000

    ep_chassis.move(x=0, y=0, z=-90, z_speed=30).wait_for_completed()
    ep_robot.close()

    return d1, d2, d3, d4


if __name__ == "__main__":
    # ds = [0.69, 2.06, 3.45, 1.46]
    # ds = [0.48, 0.73, 1.98, 0.73]
    # ds = [0.47, 0.59, 3.11, 0.32]
    ds = get_ds()
    print(ds)
    alphas = [0, math.pi / 2, math.pi, 3 * math.pi / 2]

    N = 100

    for i in range(4):
        single_measurement(
            polygon_filename="data/lab.poly",
            output_filename="data/manifolds/m_{}.obj".format(i+1),
            N=N,
            d=ds[i],
            alpha=alphas[i],
            manifold_image="data/images/m_{}.png".format(i+1)
        )

    run_mia(N, 1.0, 2.0)