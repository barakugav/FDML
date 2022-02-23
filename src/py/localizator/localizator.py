#!/usr/bin/env python3

import time
import sys
import os
import subprocess
import random


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

    def run(self):
        self.daemon_id = random.randint(0, 2**64)
        daemon_working_dir = os.path.join(self.working_dir, str(self.daemon_id))

        os.makedirs(daemon_working_dir, exist_ok=True)
        self.cmdfile = os.path.join(daemon_working_dir, ".cmdfile")
        self.ackfile = os.path.join(daemon_working_dir, ".ackfile")
        self.outfile = os.path.join(daemon_working_dir, ".outfile")
        self.logfile = open(os.path.join(daemon_working_dir, ".logfile"), "a")

        cmd = ["build/debug/robo_local_daemon", "--cmdfile",
               self.cmdfile, "--ackfile", self.ackfile]
        self.daemon = subprocess.Popen(
            cmd, stdout=self.logfile,  stderr=self.logfile)

    def stop(self):
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
        print(cmd)
        if not self.daemon:
            raise ValueError("invalid state: localizator is not running")
        print("before cmd os.path.isfile(self.ackfile)",
              os.path.isfile(self.ackfile))
        print("before cmd os.path.isfile(self.ackfile)",
              os.path.isfile(self.ackfile))
        print("before cmd os.path.isfile(self.ackfile)",
              os.path.isfile(self.ackfile))
        print("before cmd os.path.isfile(self.ackfile)",
              os.path.isfile(self.ackfile))
        print("before cmd os.path.isfile(self.ackfile)",
              os.path.isfile(self.ackfile))
        with open(self.cmdfile, "w") as cmdfile:
            cmdfile.write(cmd)
        print("os.path.isfile(self.ackfile)", os.path.isfile(self.ackfile))
        print("os.path.isfile(self.ackfile)", os.path.isfile(self.ackfile))
        print("os.path.isfile(self.ackfile)", os.path.isfile(self.ackfile))
        print("os.path.isfile(self.ackfile)", os.path.isfile(self.ackfile))
        print("os.path.isfile(self.ackfile)", os.path.isfile(self.ackfile))
        print("os.path.isfile(self.ackfile)", os.path.isfile(self.ackfile))
        while not os.path.isfile(self.ackfile):
            print("os.path.isfile(self.ackfile)", os.path.isfile(self.ackfile))
            time.sleep(0.1)  # 0.1 sec
            print("os.path.isfile(self.ackfile)", os.path.isfile(self.ackfile))
        os.remove(self.cmdfile)
        ret = None
        with open(self.ackfile, "r") as ackfile:
            ret = int(ackfile.read().strip())
            print("ret = ", ret)
        print("os.remove(self.ackfile)")
        os.remove(self.ackfile)
        print("after remove os.path.isfile(self.ackfile)",
              os.path.isfile(self.ackfile))
        print("ret = ", ret)
        if ret != 0:
            print(
                "Error during command execution. Full log can be found at", self.logfile.name)
        return ret

    def init(self, scene_filename):
        self._exec_cmd("--cmd init --scene {}".format(scene_filename))

    def query1(self, d):
        self._exec_cmd("--cmd query1 --d {} --out {}".format(d, self.outfile))

    def query2(self, d1, d2):
        self._exec_cmd(
            "--cmd query2 --d1 {} --d2 {} --out {}".format(d1, d2, self.outfile))


if __name__ == "__main__":
    localizator = Localizator(".localizator")
    localizator.run()
    localizator.init("scene01.json")
    localizator.init("scene02.json")
    localizator.query1(6)
    localizator.query1(7)
    localizator.query1(8)
    localizator.query2(9, 10)
    localizator.query1(11)
    localizator.init("scene03.json")
    localizator.query1(12)
    localizator.query2(13, 14)
    localizator.query1(15)
    localizator.stop()
