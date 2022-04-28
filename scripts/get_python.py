#!/usr/bin/env python3

import subprocess
import shutil
import re


PYTHON_VER_REQUIRED = (3, 8, 0)


def get_exe():
    path = shutil.which("python3")
    if path is not None:
        return path
    path = shutil.which("python")
    if path is not None:
        return path
    return None


def get_version(python_exe):
    out = subprocess.check_output([python_exe, "--version"]).decode("UTF-8")
    ver_str = re.findall("([0-9]+)\.([0-9]+)\.([0-9]+)", out)[0]
    return tuple([int(x) for x in ver_str])


def is_numpy_enable(python_exe):
    p = subprocess.Popen([python_exe, "-m", "pip", "show", "numpy"],
                         stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    out, err = p.communicate()
    if p.returncode != 0:
        return False
    return "not found" not in out.decode("UTF-8")


def install_numpy(python_exe):
    p = subprocess.Popen([python_exe, "-m", "pip", "install",
                         "numpy"], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    p.communicate()
    if p.returncode != 0 or not is_numpy_enable(python_exe):
        raise ValueError("Numpy installation failed")
