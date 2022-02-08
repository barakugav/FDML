#!/usr/bin/env python3

import numpy as np
import matplotlib.pyplot as plt
import math
import random
import sys

mm = [sys.float_info.max, sys.float_info.min, sys.float_info.max, sys.float_info.min]

def find_inter(m1, b1, m2, b2):
	if m1 == m2:
		raise ValueError()
	x = (b2 - b1) / (m1 - m2)
	y = x * m1 + b1
	return (x, y)

def dis(p1, p2):
	return ((p1[0] - p2[0])**2 + (p1[1] - p2[1])**2)**0.5

def res(m1 ,b1, m2, b2, d1, d2, theta):
	#k = random.uniform(0, 1)
	#d = d1 + d2
	#d1 = d * k
	#d2 = d * (1-k)

	x = ((d1 + d2) * (math.sin(theta) - m2 * math.cos(theta)) + b2 - b1) / (m1 - m2)
	y = x * m1 + b1

	x = x - d1 * math.cos(theta)
	y = y - d1 * math.sin(theta)

	p = (x, y)
	m = math.tan(theta)
	b = y - m * x

	p1 = find_inter(m, b, m1 ,b1)
	p2 = find_inter(m, b, m2, b2)
	q1 = dis(p, p1)
	q2 = dis(p, p2)
	mm[0] = min(mm[0], q1)
	mm[1] = max(mm[1], q1)
	mm[2] = min(mm[2], q2)
	mm[3] = max(mm[3], q2)
	#if x < -20:
	#	print((x, y, theta), p1, p2, q1, q2)

	return (x, y, theta)

def raa(m1, b1, m2, b2, d1, d2, theta_limit):
	ps = []
	for _ in range(10000):
		theta = random.uniform(theta_limit[0], theta_limit[1])
		p = res(m1 ,b1, m2, b2, d1, d2, theta)
		ps.append(p)
	ps = np.array(ps)
	return ps[ps[:,0].argsort()]

def calc_ellipse(m1, b1, m2, b2, d1, d2, x, side):
	raise ValueError()

	d = d1
	x0, y0 = find_inter(m1, b1, m2, b2)
	t = 0
	alpha = math.atan(m1)
	a = d / math.tan(alpha)
	b = d / math.tan(math.pi/2 - alpha)

	if 1-(x-x0)**2/a**2 >= 0:
		y = y0 + (1 if side else -1) * (b * (1- (x-x0)**2/a**2)**0.5)
	else:
		y = min(m1*x+b1, b2*x+b2)

	return y

from matplotlib.patches import Ellipse

def plot_ellipse(m1, b1, m2, b2, d1, d2):

	x0, y0 = find_inter(m1, b1, m2, b2)
	alpha = math.atan(m1) - math.atan(m2)
	beta = (d1 / (d1 + d2)) * alpha
	# beta = (4 * ((d1 / (d1 + d2)) - 0.5)**3 + 0.5) * alpha
	# beta = (math.tan(d1 / (d1 + d2) -0.5) / (2 * math.tan(0.5)) + 0.5) *alpha
	# beta = (math.atan(d1 / (d1+d2) -0.5) / (2*math.atan(0.5)) +0.5) *alpha
	# beta = (math.atan(d1 / (d1+d2) -0.5) +0.5) *alpha
	# beta = (math.atan((d1 / (d1+d2) -0.5) *2 * math.tan(0.5)) +0.5) *alpha
	beta = math.atan(d1 / d2) * 2 / math.pi
	t = math.atan(m2) + alpha - beta
	# a = d1 * math.sin(math.pi/2 - alpha + beta) / math.sin(beta)
	a = 8.5
	# a = d1 * math.cos(alpha - beta) / math.sin(beta)
	b = 0.4

	# d =d1
	# alpha = math.atan(m1) - t
	# z = (d1 + d2) / (2 * math.tan(alpha))
	# a = (z**2 + (d2 - d1)**2)**0.5
	# a = z
	# print(z,a)
	# t = t + math.atan2(d2 - d1, z) / 2
	# t = t + math.atan((d2-d1)/z)
	
	# b = d / math.tan(math.pi/2 - alpha)
	# print(alpha, a, b)

	e = Ellipse((x0, y0), a*2, b*2, math.degrees(t))
	a = plt.subplot(111, aspect='equal')
	e.set_clip_box(a.bbox)
	e.set_alpha(0.1)
	a.add_artist(e)
	# plt.xlim(-25, -4)
	# plt.ylim(6, 10)
	

if __name__ == '__main__':
	m1, b1 = 0.05, 10
	m2, b2 = -1, 5
	d1, d2 = 0.5, 3.5
	d1, d2 = 1, 3
	# d1, d2 = 0.01, 3.99
	#d1, d2 = 0.1, 3.9
	# d1, d2 = 2, 2

	theta_limit = [math.atan2(m1,1), math.pi + math.atan2(m2, 1)]
	# print("bottom slope and cell angle interval", m2, theta_limit)

	# <= perpendicular
	theta_limit = [math.atan2(m1,1), math.atan2(m1,1) + math.pi/2]
	ps1 = raa(m1, b1, m2, b2, d1, d2, theta_limit)
	# >= perpendicular
	theta_limit = [math.atan2(m1,1) + math.pi/2, math.atan2(m2,1) + math.pi]
	ps2 = raa(m1, b1, m2, b2, d1, d2, theta_limit)

	#print(mm)

	xs = np.concatenate((ps1[:,0], ps2[:,0]))
	top = [x * m1 + b1 for x in xs]
	bottom = [x * m2 + b2 for x in xs]
	res1 = ps1[:,1]
	res2 = ps2[:,1]

	#print(ps1[len(ps1)-1][2])

	# xs = np.linspace(-20,20,10000)
	# ellipse1 = [calc_ellipse(m1, b1, m2, b2, d1, d2, x, True) for x in xs]
	# ellipse2 = [calc_ellipse(m1, b1, m2, b2, d1, d2, x, False) for x in xs]
	# print(np.amax(ellipse1), np.amin(ellipse1))
	# print(np.amax(ellipse2), np.amin(ellipse2))

	#for i in range(10):
	#	print([xs[i], top[i], bottom[i], res[i]])

	a = plt.subplot(111, aspect='equal')
	plt.plot(xs, top, label="top")
	plt.plot(xs, bottom, label="bottom")
	plt.plot(ps1[:,0], res1, label="<= perpendicular")
	plt.plot(ps2[:,0], res2, label=">= perpendicular")
	# plt.plot(xs, ellipse1, label="ellipse")
	# plt.plot(xs, ellipse2, label="ellipse")
	plot_ellipse(m1, b1, m2, b2, d1, d2)
	# plt.legend();
	fig = plt.gcf()
	fig.set_size_inches(18.5, 10.5)
	plt.savefig("temp.png")
	plt.show()
	plt.clf()
