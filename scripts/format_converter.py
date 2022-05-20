#!/usr/bin/env python3

from enum import Enum
import json
import argparse


class FormatType(Enum):
    Json = 1
    Mickel = 2
    Efi = 3

    @staticmethod
    def from_string(str):
        if str == "json":
            return FormatType.Json
        elif str == "mickel":
            return FormatType.Mickel
        elif str == "efi":
            return FormatType.Efi
        else:
            raise ValueError("unknown string:", str)

    def to_string(self):
        if self == FormatType.Json:
            return "json"
        elif self == FormatType.Mickel:
            return "mickel"
        elif self == FormatType.Efi:
            return "efi"
        else:
            raise ValueError("unknown self type")


class Scene:
    def __init__(self, boundary, holes):
        self.boundary = boundary
        self.holes = holes


def parse_scene_from_file(filename, format):
    if format == FormatType.Json:
        with open(filename, "r") as in_file:
            data = json.load(in_file)
        boundary = data["scene_boundary"]
        holes = data.get("holes")
        return Scene(boundary, holes)

    elif format == FormatType.Mickel:
        with open(filename, "r") as in_file:
            lines = in_file.readlines()
        boundary = []
        for line in lines:
            l = line.split()
            x, y = l[0], l[1]
            x, y = float(x), float(y)
            boundary.append((x, y))
        return Scene(boundary, [])

    elif format == FormatType.Efi:
        with open(filename, "r") as in_file:

            def read_polygon(input):
                pgn = []
                n = int(input.readline())
                for _ in range(n):
                    line = input.readline().split()
                    pgn.append((float(line[0]), float(line[1])))
                return pgn

            boundary = read_polygon(in_file)
            holes = []

            holes_num = int(in_file.readline())
            for _ in range(holes_num):
                holes.append(read_polygon(in_file))
        return Scene(boundary, holes)

    else:
        raise ValueError("unknown format:", format)


def write_scene_to_file(scene, filename, format):
    if format == FormatType.Json:
        json_string = {
            "scene_boundary": scene.boundary,
            "holes": scene.holes,
        }
        with open(filename, "w") as outfile:
            json.dump(json_string, outfile)

    elif format == FormatType.Mickel:
        with open(filename, "w") as outfile:
            for p in scene.boundary:
                outfile.write(f"{p[0]} {p[1]}\n")

    elif format == FormatType.Efi:
        with open(filename, "w") as outfile:
            outfile.write(f"{len(scene.boundary)}\n")
            for p in scene.boundary:
                outfile.write(f"{p[0]} {p[1]}\n")
            outfile.write(f"{len(scene.holes)}\n")
            for hole in scene.holes:
                outfile.write(f"{len(hole)}\n")
                for p in hole:
                    outfile.write(f"{p[0]} {p[1]}\n")

    else:
        raise ValueError("unknown format:", format)


if __name__ == "__main__":
    format_types_str = [fmt.to_string() for fmt in list(FormatType)]

    parser = argparse.ArgumentParser(description="Scene format converter")
    parser.add_argument("--source-file", required=True,
                        type=str, help="path to source file")
    parser.add_argument("--source-fmt", required=True,
                        choices=format_types_str, help="source format")
    parser.add_argument("--target-file", required=True,
                        type=str, help="path to target file")
    parser.add_argument("--target-fmt", required=True,
                        choices=format_types_str, help="target format")
    args = parser.parse_args()

    scene = parse_scene_from_file(
        args.source_file, FormatType.from_string(args.source_fmt))
    write_scene_to_file(scene, args.target_file,
                        FormatType.from_string(args.target_fmt))
