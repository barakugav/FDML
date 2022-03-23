#!/usr/bin/env python3

import os
import sys
import argparse

TOOL_DIR = os.path.dirname(os.path.realpath(__file__))
COMMON_DIR = os.path.join(TOOL_DIR, "..", "common")

if COMMON_DIR not in sys.path:
    sys.path.insert(0, COMMON_DIR)
import build_tool


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="FDML Daemon builder")
    parser.add_argument("--build-type", dest="build_type", type=str,
                        choices=["debug", "release"], default="debug", help="Type of the build")
    args = parser.parse_args()

    debug = args.build_type == "debug"

    build_tool.build_tool(TOOL_DIR, debug=debug)
