#!/usr/bin/env python3

import time
import os
import subprocess


class Localizator:
    def __init__(self, working_dir):
        self.working_dir = working_dir
        self.cmdfile = os.path.join(working_dir, ".cmdfile")
        self.ackfile = os.path.join(working_dir, ".ackfile")
        self.outfile = os.path.join(working_dir, ".outfile")
        self.is_running = False
        self.daemon = None

    def run(self):
        os.makedirs(self.working_dir, exist_ok=True)
        cmd = ["build/debug/robo_local_daemon", "--cmdfil",
               self.cmdfile, "--ackfile", self.ackfile]
        self.daemon = subprocess.Popen(
            cmd, stdout=subprocess.PIPE,  stderr=subprocess.STDOUT)

    def stop(self):
        if not self.daemon:
            return
        self.daemon.kill()
        self.daemon = None

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
            print("error during command execution")
        return ret

    def init(self, scene_filename):
        self._exec_cmd("--cmd init --scene {}".format(scene_filename))

    def query1(self, d):
        self._exec_cmd("--cmd query1 --d {}".format(d))

    def query2(self, d1, d2):
        self._exec_cmd("--cmd query2 --d1 {} --d2 {}".format(d1, d2))


if __name__ == "__main__":
    localizator = Localizator(".localizator")
    localizator.run()
    localizator.init("scene01.json")
    localizator.init("scene01.json")
    localizator.query1(6)
    localizator.query1(7)
    localizator.query1(9)
    localizator.query2(9, 10)
    localizator.query1(8)
    localizator.init("scene02.json")
    localizator.query1(9)
    localizator.query2(9, 10)
    localizator.query1(8)
