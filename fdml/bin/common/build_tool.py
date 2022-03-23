#!/usr/bin/env python3

import os
import sys
import subprocess


def build_tool(tool_dir, debug=True, parallel=4):
    if not (sys.platform == "linux" or sys.platform == "linux2"):
        raise ValueError(
            "Only Linux is supported by this script. For windows, build using Visual Studio. See README.md")
    build_dir = os.path.join(
        tool_dir, "build", "debug" if debug else "release")
    os.makedirs(build_dir, exist_ok=True)
    if not os.path.exists(os.path.join(build_dir, "Makefile")):
        subprocess.run(["cmake", "-DCMAKE_BUILD_TYPE=" +
                       ("Debug" if debug else "Release"), "-B" + build_dir, "-S" + tool_dir])
    if not parallel or parallel == 1:
        subprocess.run(["make", "-C", build_dir])
    else:
        subprocess.run(["make", "-C", build_dir, "-j" + str(parallel)])
