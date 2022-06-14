import time
import math
import matplotlib.pyplot as plt

import robomaster
from robomaster import robot, led

dist_buffer_front = []
dist_buffer_back = []

def cache_distance(dist):
    """
    Distance measurement callback. Caches both sensors into the global buffers
    """
    print(dist)

if __name__ == "__main__":
    # Connect to robot
    ep_robot = robot.Robot()
    ep_robot.initialize(conn_type='ap')
    
    # Fetch modules
    ep_chassis = ep_robot.chassis
    ep_led = ep_robot.led
    ep_sensor = ep_robot.sensor
    ep_sensor.sub_distance(callback=cache_distance)

    # Add cool colors to the robot
    ep_led.set_led(comp=led.COMP_BOTTOM_BACK, r=147, g=204, b=234, effect=led.EFFECT_ON) # Back
    ep_led.set_led(comp=led.COMP_BOTTOM_FRONT, r=255, g=0, b=193, effect=led.EFFECT_ON) # Right
    ep_led.set_led(comp=led.COMP_BOTTOM_LEFT, r=255, g=0, b=193, effect=led.EFFECT_ON) # Left
    ep_led.set_led(comp=led.COMP_BOTTOM_RIGHT, r=255, g=255, b=255, effect=led.EFFECT_ON) # Front
    
    time.sleep(60 * 10)

    # We are now done with the robot
    ep_robot.close()
