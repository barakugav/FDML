#!/usr/bin/env python3

import json
import random
from enum import IntEnum


class Direction(IntEnum):
    Xp = 0,
    Xn = 1,
    Yp = 2,
    Yn = 3

    def opposite(self):
        if self == Direction.Xp:
            return Direction.Xn
        elif self == Direction.Xn:
            return Direction.Xp
        elif self == Direction.Yp:
            return Direction.Yn
        elif self == Direction.Yn:
            return Direction.Yp
        raise ValueError()

    def is_positive(self):
        return self == Direction.Xp or self == Direction.Yp


class Position:
    def __init__(self, x, y):
        self.x = x
        self.y = y

    def add(self, direction):
        if direction == Direction.Xp:
            return Position(self.x + 1, self.y)
        if direction == Direction.Xn:
            return Position(self.x - 1, self.y)
        if direction == Direction.Yp:
            return Position(self.x, self.y + 1)
        if direction == Direction.Yn:
            return Position(self.x, self.y - 1)
        raise ValueError("Unknown direction ", direction)

    @staticmethod
    def distance(p1, p2):
        return abs(p1.x - p2.x) + abs(p1.y - p2.y)

    def _cmp(self, other):
        if self.x != other.x:
            return -1 if self.x < other.x else 1
        if self.y != other.y:
            return -1 if self.y < other.y else 1
        return 0

    def __eq__(self, other):
        return self._cmp(other) == 0 if isinstance(other, Position) else False

    def __ne__(self, other):
        return self._cmp(other) != 0 if isinstance(other, Position) else True

    def __gt__(self, other):
        return self._cmp(other) > 0

    def __ge__(self, other):
        return self._cmp(other) >= 0

    def __le__(self, other):
        return self._cmp(other) <= 0

    def __lt__(self, other):
        return self._cmp(other) < 0

    def __key(self):
        return (self.x, self.y)

    def __hash__(self):
        return hash(self.__key())

    def __repr__(self):
        return "(%d, %d)" % (self.x, self.y)


class RobotPath:
    def __init__(self, start, goal):
        self.start = start
        self.goal = goal

    def __repr__(self):
        return "{%s, %s}" % (self.start, self.goal)


class SceneDescription:
    def __init__(self, width, height, paths):
        self.width = width
        self.height = height
        self.paths = paths


def write_scene_to_file(scene, filename):
    if not isinstance(scene, SceneDescription):
        raise ValueError("passed object is not a scene", type(scene))

    def Position_to_jsonobj(o):
        return [o.x, o.y]

    def RobotPath_to_jsonobj(o):
        return [Position_to_jsonobj(o.start), Position_to_jsonobj(o.goal)]
    scene_obj = {
        "width": scene.width, "height": scene.height,
        "paths": [RobotPath_to_jsonobj(o) for o in scene.paths]
    }
    json_string = json.dumps(scene_obj)
    with open(filename, "w") as outfile:
        outfile.write(json_string)


def parse_scene_from_file(filename):
    with open(filename, "r") as infile:
        data = json.load(infile)

    def jsonobj_to_Position(o):
        return Position(o[0], o[1])

    def jsonobj_to_RobotPath(o):
        return RobotPath(jsonobj_to_Position(o[0]), jsonobj_to_Position(o[1]))

    width, height = data["width"], data["height"]
    paths = [jsonobj_to_RobotPath(p) for p in data["paths"]]
    return SceneDescription(width, height, paths)


def generate_random_scene(width, height, robot_num):
    if robot_num >= (width + height) * 0.8:
        raise ValueError("Too many robot for a small scene")

    def order_range(a, b):
        return (a, b) if a <= b else (b, a)

    paths = []
    used_start_pos = set()
    used_columns = set()
    used_rows = set()

    for _ in range(robot_num * 100):
        # generate start position
        for _ in range(1000):
            start_pos = Position(random.randrange(
                width), random.randrange(height))
            if start_pos in used_start_pos:
                continue
            if start_pos.x in used_columns and start_pos.y in used_rows:
                continue
            break
        else:
            break

        vertical_choices = []
        if start_pos.x not in used_columns:
            vertical_choices.append(True)
        if start_pos.y not in used_rows:
            vertical_choices.append(False)

        # generate goal position
        for _ in range(1000):
            is_vertical = random.choice(vertical_choices)
            if is_vertical:
                goal_pos = Position(start_pos.x, random.randrange(height))
            else:
                goal_pos = Position(random.randrange(width), start_pos.y)

            collide = False
            for path in paths:
                if goal_pos.x == path.start.x and goal_pos.x == path.goal.x:
                    y1, y2 = order_range(path.start.y, path.goal.y)
                    if y1 <= goal_pos.y and goal_pos.y <= y2:
                        collide = True

                elif goal_pos.y == path.start.y and goal_pos.y == path.goal.y:
                    x1, x2 = order_range(path.start.x, path.goal.x)
                    if x1 <= goal_pos.x and goal_pos.x <= x2:
                        collide = True

                if collide:
                    break
            if not collide:
                break
        else:
            continue

        paths.append(RobotPath(start_pos, goal_pos))
        used_start_pos.add(start_pos)
        used_columns.add(start_pos.x)
        used_rows.add(start_pos.y)
        if len(paths) >= robot_num:
            break
    else:
        raise ValueError(
            "Failed to find another location valid path. Current configuration: %s" % paths)
    return SceneDescription(width, height, paths)


if __name__ == "__main__":
    pass
