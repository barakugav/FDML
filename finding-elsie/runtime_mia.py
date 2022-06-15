from re import sub
import os
import subprocess

def generate_command_str(first, second, third, delta, radius):
    return 'bin/mia --m1-path=data/manifolds/m_{}.obj --m2-path=data/manifolds/m_{}.obj --mout-path=data/manifolds/m_{}.obj --radius={} --delta={}'.format(first, second, third, radius, delta).split()

def run_mia(radius, delta1=0.5, delta2=2.0):
    subprocess.Popen(generate_command_str(1, 2, 12, delta1, radius)).wait()
    subprocess.Popen(generate_command_str(3, 4, 34, delta1, radius)).wait()
    subprocess.Popen(generate_command_str(12, 34, 1234, delta2, radius)).wait()

if __name__ == "__main__":
    run_mia(150, delta1=1.0, delta2=2.0)