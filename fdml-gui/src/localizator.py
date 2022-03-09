#!/usr/bin/env python3

import time
import os
import sys
import subprocess
import random
import json
import numpy as np
import threading
import atexit
import tempfile
import shutil

FDML_CORE_TOP = os.path.abspath(os.path.join(
    os.path.dirname(os.path.realpath(__file__)), "../../fdml/"))

DEBUG_EN = True
DAEMON_PATH_LINUX = os.path.join(
    FDML_CORE_TOP, "build/linux", "debug" if DEBUG_EN else "release", "fdml_daemon")
DAEMON_PATH_WINDOWS = os.path.join(
    FDML_CORE_TOP, "build/win", "Debug" if DEBUG_EN else "Release", "fdml_daemon")
DAEMON_PATH = DAEMON_PATH_LINUX if sys.platform == "linux" or sys.platform == "linux2" else DAEMON_PATH_WINDOWS

daemons = set()
daemons_lock = threading.Lock()


def kill_all_daemons():
    with daemons_lock:
        for deamon in daemons:
            deamon.kill()
        daemons.clear()


atexit.register(kill_all_daemons)


class Localizator:
    def __init__(self):
        self.working_dir = tempfile.mkdtemp()
        self.daemon_id = None
        self.cmdfile = None
        self.ackfile = None
        self.querynum = 0
        self.logfile = None
        self.is_running = False
        self.daemon = None
        self.lock = threading.Lock()

    def __del__(self):
        with self.lock:
            if self.daemon:
                print("Localizator was not stopped! terminating.")
                self.daemon.kill()
        shutil.rmtree(self.working_dir)

    def _working_dir(self):
        return os.path.join(self.working_dir, str(self.daemon_id))

    def run(self, scene_filename):
        with self.lock:
            self.daemon_id = random.randint(0, 2**64)
            working_dir = self._working_dir()

            os.makedirs(working_dir, exist_ok=True)
            self.cmdfile = os.path.join(working_dir, ".cmdfile")
            self.ackfile = os.path.join(working_dir, ".ackfile")
            self.logfile = open(os.path.join(
                working_dir, ".logfile"), "a")

            cmd = [DAEMON_PATH, "--cmdfile",
                   self.cmdfile, "--ackfile", self.ackfile]
            self.daemon = subprocess.Popen(
                cmd, stdout=self.logfile,  stderr=self.logfile)
            with daemons_lock:
                daemons.add(self.daemon)

            self._exec_cmd("--cmd init --scene {}".format(scene_filename))

    def stop(self):
        with self.lock:
            if not self.daemon:
                return
            self._exec_cmd("--cmd quit")
            try:
                self.daemon.wait(1) # wait 1 sec
            except subprocess.TimeoutExpired as e:
                print("Daemon failed to quit, terminating.")
                self.daemon.terminate()
                self.daemon.kill()
            with daemons_lock:
                daemons.remove(self.daemon)
            self.daemon = None
            self.daemon_id = None
            self.cmdfile = None
            self.ackfile = None
            self.logfile.close()
            self.logfile = None

    def _exec_cmd(self, cmd):
        if not self.daemon:
            raise ValueError("invalid state: localizator is not running")
        with open(self.cmdfile, "w") as cmdfile:
            cmdfile.write(cmd)
        while not os.path.isfile(self.ackfile):
            time.sleep(0.1)  # 0.1 sec
        os.remove(self.cmdfile)
        with open(self.ackfile, "r") as ackfile:
            ret = int(ackfile.read().strip())
        os.remove(self.ackfile)
        if ret != 0:
            raise ValueError(
                "Error during command execution. Full log can be found at", self.logfile.name)

    def _next_outfile(self):
        self.querynum += 1
        return os.path.join(self._working_dir(), ".outfile{}".format(self.querynum))

    def _exec_cmd_and_read_res(self, cmd, outfile):
        try:
            self._exec_cmd(cmd)
            try:
                with open(outfile, "r") as outf:
                    return json.load(outf)
            except Exception as e:
                print("Failed to read result file", outfile)
                raise e
        finally:
            if os.path.isfile(outfile):
                os.remove(outfile)

    def query1(self, d):
        with self.lock:
            outfile = self._next_outfile()
            data = self._exec_cmd_and_read_res(
                "--cmd query1 --d {} --out {}".format(d, outfile), outfile)
            return [np.array(polygon) for polygon in data["polygons"]]

    def query2(self, d1, d2):
        with self.lock:
            outfile = self._next_outfile()
            data = self._exec_cmd_and_read_res(
                "--cmd query2 --d1 {} --d2 {} --out {}".format(d1, d2, outfile), outfile)
            return data["segments"]
