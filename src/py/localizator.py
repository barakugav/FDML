#!/usr/bin/env python3

import time
import os
import subprocess
import random
import json
import numpy as np
import threading


class Localizator:
    def __init__(self, working_dir):
        self.working_dir = working_dir
        self.daemon_id = None
        self.cmdfile = None
        self.ackfile = None
        self.outfile = None
        self.logfile = None
        self.is_running = False
        self.daemon = None
        self.lock = threading.Lock()

    def run(self, scene_filename):
        with self.lock:
            self.daemon_id = random.randint(0, 2**64)
            daemon_working_dir = os.path.join(
                self.working_dir, str(self.daemon_id))

            print(daemon_working_dir)
            os.makedirs(daemon_working_dir, exist_ok=True)
            self.cmdfile = os.path.join(daemon_working_dir, ".cmdfile")
            self.ackfile = os.path.join(daemon_working_dir, ".ackfile")
            self.outfile = os.path.join(daemon_working_dir, ".outfile")
            self.logfile = open(os.path.join(daemon_working_dir, ".logfile"), "a")

            cmd = ["build/debug/robo_local_daemon", "--cmdfile",
            # cmd = ["C:\\projects\\university\\algorithmic_robotics_and_motion_planning\\project\\build\\win\\Debug\\robo_local_daemon.exe", "--cmdfile",
                self.cmdfile, "--ackfile", self.ackfile]
            self.daemon = subprocess.Popen(
                cmd, stdout=self.logfile,  stderr=self.logfile)

            self._exec_cmd("--cmd init --scene {}".format(scene_filename))

    def stop(self):
        with self.lock:
            if not self.daemon:
                return
            self.daemon.kill()
            self.daemon = None
            self.daemon_id = None
            self.cmdfile = None
            self.ackfile = None
            self.outfile = None
            self.logfile.close()
            self.logfile = None

    def _exec_cmd(self, cmd):
        print(cmd) # TODO remove
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
            if os.path.isfile(self.outfile):
                os.remove(self.outfile)
            raise ValueError(
                "Error during command execution. Full log can be found at", self.logfile.name)

    def query1(self, d):
        with self.lock:
            self._exec_cmd("--cmd query1 --d {} --out {}".format(d, self.outfile))
            try:
                with open(self.outfile, "r") as outfile:
                    data = json.load(outfile)
                polygons = []
                polygons_json = data["polygons"]
                for polygon in polygons_json:
                    polygons.append(np.array(polygon))
                return polygons
            except Exception as e:
                print("Failed to read result file", self.outfile)
                raise e
            finally:
                if os.path.isfile(self.outfile):
                    os.remove(self.outfile)

    def query2(self, d1, d2):
        with self.lock:
            self._exec_cmd(
                "--cmd query2 --d1 {} --d2 {} --out {}".format(d1, d2, self.outfile))


if __name__ == "__main__":
    localizator = Localizator(os.path.join(os.path.dirname(os.path.realpath(__file__)), ".localizator"))
    localizator.run("scene01.json")
    # localizator.run("C:\\projects\\university\\algorithmic_robotics_and_motion_planning\\project\\scene01.json")
    localizator.query1(6)
    # localizator.query1(7)
    # localizator.query1(8)
    # localizator.query2(9, 10)
    # localizator.query1(11)
    localizator.stop()
