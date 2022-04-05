#!/usr/bin/python3
# export PYTHONPATH=...

import os
import sys
import importlib
import timeit
import fdml

if len(sys.argv) < 2:
    print('Library name missing, assuming CGALPY')
    sys.path.append(os.path.abspath('../precompiled'))
    lib = 'CGALPY'
else:
    lib = sys.argv[1]
CGALPY = importlib.import_module(lib)

Locator = fdml.Locator
ker = CGALPY.Ker
Point = ker.Point_2
FT = ker.FT
l = Locator()
Polygon = fdml.pol2.Polygon_2
boundary = Polygon([Point(-2, -2), Point(2, -2), Point(2, 2), Point(-2, 2)])
l.init(boundary)
pgns = l.query1(FT(1))
print(len(pgns))

PS = CGALPY.Bso2.Polygon_set_2
ps = PS()
ps.insert(pgns, [])
arr = ps.arrangement()
print(arr.number_of_vertices(), arr.number_of_halfedges(), arr.number_of_faces())
