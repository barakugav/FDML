#!/usr/bin/env python3

import sys
import random
from sortedcontainers import SortedList
import argparse


class Position:
    def __init__(self, x, y):
        self.x = x
        self.y = y

    # def __eq__(self, other):
    #     if not isinstance(other, Position):
    #         return False
    #     return self.x == other.x and self.y == other.y

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


class RobotStartGoal:
    def __init__(self, start, goal):
        self.start = start
        self.goal = goal

    def __repr__(self):
        return "{%s, %s}" % (self.start, self.goal)


class Scene:
    def __init__(self, n, m, paths):
        self.n = n
        self.m = m
        self.paths = paths


def print_scene_file_format():
    raise ValueError("not supported")
    print("""
""")


def parse_scene_from_file(filename):
    raise ValueError("not supported")


def order_range(a, b):
    return (a, b) if a <= b else (b, a)


def sign(x):
    if x < 0:
        return -1
    if x > 0:
        return 1
    return 0


def is_paths_intersect(p1, p2):
    x11, x12 = order_range(p1[0].x, p1[1].x)
    y11, y12 = order_range(p1[0].y, p1[1].y)
    x21, x22 = order_range(p2[0].x, p2[1].x)
    y21, y22 = order_range(p2[0].y, p2[1].y)
    return x11 <= x22 and x12 >= x21 and y11 <= y22 and y12 >= y21


def generate_random_scene(n, m, k):
    if k >= (n + m) * 0.8:
        raise ValueError("Too many robot for a small scene")

    paths = []
    used_start_pos = set()
    used_columns = set()
    used_rows = set()

    for _ in range(k * 100):
        # generate start position
        for _ in range(1000):
            start_pos = Position(random.randrange(n), random.randrange(m))
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
                goal_pos = Position(start_pos.x, random.randrange(m))
            else:
                goal_pos = Position(random.randrange(n), start_pos.y)

            collide = False
            for robot in paths:
                if goal_pos.x == robot.start.x and goal_pos.x == robot.goal.x:
                    y1, y2 = order_range(robot.start.y, robot.goal.y)
                    if y1 <= goal_pos.y and goal_pos.y <= y2:
                        collide = True

                elif goal_pos.y == robot.start.y and goal_pos.y == robot.goal.y:
                    x1, x2 = order_range(robot.start.x, robot.goal.x)
                    if x1 <= goal_pos.x and goal_pos.x <= x2:
                        collide = True

                if collide:
                    break
            if not collide:
                break
        else:
            continue

        paths.append(RobotStartGoal(start_pos, goal_pos))
        used_start_pos.add(start_pos)
        used_columns.add(start_pos.x)
        used_rows.add(start_pos.y)
        if len(paths) >= k:
            break
    else:
        raise ValueError(
            "Failed to find another location valid path. Current configuration: %s" % paths)
    return Scene(n, m, paths)


class Robot:
    def __init__(self, id, start, goal):
        self.id = id
        self.start = start
        self.goal = goal
        self.pos = start

        self.succr_xp = None
        self.succr_xn = None
        self.succr_yp = None
        self.succr_yn = None

    def __repr__(self):
        return "{%s, %s, %s}" % (self.pos, self.start, self.goal)


class Board:
    def __init__(self, scene):
        self.n = scene.n
        self.m = scene.m
        self.robots = [Robot(id, r.start, r.goal)
                       for id, r in enumerate(scene.paths)]


def print_board(board, print_successors=False):
    robots = sorted(
        board.robots, key=lambda robot: robot.pos.x + robot.pos.y * board.n)

    def pos_next(pos):
        return Position(pos.x + 1, pos.y) if pos.x < board.n - 1 else Position(0, pos.y+1)

    def print_empty(from_pos, to_pos):
        while from_pos != to_pos:
            print("|  ", end="")
            if from_pos.x == board.n - 1:
                print("|")
                print("-" * (board.n * 3 + 1))
            from_pos = pos_next(from_pos)

    printed_until = Position(0, 0)
    print("-" * (board.n * 3 + 1))
    for robot in robots:
        print_empty(printed_until, robot.pos)
        print("|{:02}".format(robot.id), end="")
        if robot.pos.x == board.n - 1:
            print("|")
            print("-" * (board.n * 3 + 1))
        printed_until = pos_next(robot.pos)
    print_empty(printed_until, Position(0, board.m))

    if print_successors:
        print(" ID: X+ X- Y+ Y-")
        for robot in robots:
            successors = [robot.succr_xp, robot.succr_xn,
                          robot.succr_yp, robot.succr_yn]
            ids = [
                "--" if r is None else "{:02}".format(r.id) for r in successors]
            print("R{:02}: {} {} {} {} {}".format(robot.id, *ids, robot))


def solve_scene(scene):
    board = Board(scene)
    moves = []

    def move_robot(robot, new_pos):
        assert robot.pos != new_pos
        assert robot.pos.x == new_pos.x
        moves.append((robot.id, robot.pos, new_pos))
        robot.pos = new_pos

    # Perform sweep to calculate successors x+,x-,y+,y- of all robots
    # Running time is O(k log k)
    board.robots.sort(key=lambda r: r.start)
    # print(board.robots) # TODO remove
    visible_robots = SortedList(key=lambda robot: robot.start.y)
    prev_col_robot = None
    for robot in board.robots:
        # Check if last robot was exactly below (negative y, yn) current robot
        if prev_col_robot is not None and robot.pos.x == prev_col_robot.pos.x:
            assert robot.pos.y > prev_col_robot.pos.y
            prev_col_robot.succr_yp = robot
            robot.succr_yn = prev_col_robot
        prev_col_robot = robot

        # Check if there is some robot exactly to the left (negative x, xn) of the current robot
        prev_row_robot_idx = visible_robots.bisect_left(robot)
        prev_row_robot = visible_robots[prev_row_robot_idx] if prev_row_robot_idx < len(
            visible_robots) else None
        prev_row_robot = prev_row_robot if prev_row_robot is not None and prev_row_robot.pos.y == robot.pos.y else None
        if prev_row_robot is not None:
            assert robot.pos.x > prev_row_robot.pos.x
            prev_row_robot.succr_xp = robot
            robot.succr_xn = prev_row_robot
            visible_robots.remove(prev_row_robot)
        visible_robots.add(robot)
    visible_robots.clear()

    print_board(board, print_successors=True)  # TODO remove

    def get_blocking_robot(robot):
        if robot.pos == robot.goal:
            return None
        if robot.pos.x == robot.goal.x:
            # column robot
            assert robot.pos.y != robot.goal.y
            if robot.pos.y < robot.goal.y:
                return None if robot.succr_yp == None or robot.goal.y < robot.succr_yp.pos.y else robot.succr_yp
            else:
                return None if robot.succr_yn == None or robot.goal.y > robot.succr_yn.pos.y else robot.succr_yn
        else:
            # row robot
            assert robot.pos.x != robot.goal.x
            if robot.pos.x < robot.goal.x:
                return None if robot.succr_xp == None or robot.goal.x < robot.succr_xp.pos.x else robot.succr_xp
            else:
                return None if robot.succr_xn == None or robot.goal.x > robot.succr_xn.pos.x else robot.succr_xn

    def has_clear_path(robot):
        return get_blocking_robot(robot) is None

    # After moving one robot, successors should be check if their path was cleared
    # Running time is O(k)
    def advance_robots_with_clear_path():
        for robot in board.robots:
            queue = [robot]
            while len(queue) > 0:
                robot = queue.pop()
                if not has_clear_path(robot):
                    continue

                # update near robots' successors attributes
                if robot.start.x == robot.goal.x:
                    # column robot
                    if robot.succr_xp is not None:
                        robot.succr_xp.succr_xn = robot.succr_xn
                        queue.append(robot.succr_xp)
                    if robot.succr_xn is not None:
                        robot.succr_xn.succr_xp = robot.succr_xp
                        queue.append(robot.succr_xn)
                else:
                    # row robot
                    if robot.succr_yp is not None:
                        robot.succr_yp.succr_yn = robot.succr_yn
                        queue.append(robot.succr_yp)
                    if robot.succr_yn is not None:
                        robot.succr_yn.succr_yp = robot.succr_yp
                        queue.append(robot.succr_yn)

                # Actual move
                move_robot(robot, robot.goal)

                # Clear successors
                robot.succr_xp = robot.succr_xn = robot.succr_yp = robot.succr_yn = None

    # Start by moving all robots with clear path to their target
    advance_robots_with_clear_path()

    # Calculate all cycles
    cycles = []
    visited = [False] * len(board.robots)
    for robot in board.robots:
        if robot.pos == robot.goal or visited[robot.id]:
            continue
        first_robot = robot
        cycle = []
        while True:
            if visited[robot.id]:
                assert robot == first_robot
                break
            visited[robot.id] = True
            cycle.append(robot)
            robot = get_blocking_robot(robot)
            assert robot is not None
        cycles.append(cycle)

    # Solve each cycle independently
    for cycle in cycles:
        assert cycle >= 4
        for idx, robot in enumerate(cycle):
            assert robot.pos != robot.goal
            next_robot = cycle[(idx + 1) % len(cycle)]
            robot_path = [robot.pos, next_robot.pos]

            intersecting_robots = []
            for idx_other, robot_other in enumerate(cycle):
                next_robot_other = cycle[(idx_other + 1) % len(cycle)]
                robot_other_path = [robot_other.pos, next_robot_other.pos]
                if is_paths_intersect(robot_path, robot_other_path):
                    intersecting_robots.append(robot_other)

            empty_pos = None
            if robot.pos.x == robot.goal.x:
                # column robot
                movement_sign = sign(robot.goal.y - robot.pos.y)
                empty_y = robot.pos.y + movement_sign
                for robot_other in sorted(intersecting_robots, key=lambda r: r.y, reverse=movement_sign < 0):
                    if empty_y != robot_other.y:
                        break
                    empty_y += movement_sign
                if empty_y != next_robot.pos.y:
                    empty_pos = Position(robot.pos.x, empty_y)

            else:
                # row robot
                movement_sign = sign(robot.goal.y - robot.pos.y)
                empty_x = robot.pos.x + movement_sign
                for robot_other in sorted(intersecting_robots, key=lambda r: r.x, reverse=movement_sign < 0):
                    if empty_x != robot_other.x:
                        break
                    empty_x += movement_sign
                if empty_x != next_robot.pos.y:
                    empty_pos = Position(empty_x, robot.pos.y)
            if empty_pos is not None:
                # Found empty position. Advance a robot to the empty position, and all other robots to their
                # cycle's target position, solving the cycle.
                prev_pos = robot.pos
                move_robot(robot, empty_pos)

                idx = (idx - 1) % len(cycle)
                while True:
                    r = cycle[idx]
                    pprev_pos = r.pos
                    move_robot(r, prev_pos)
                    prev_pos = pprev_pos
                    if r == robot:
                        break
                    idx = (idx - 1) % len(cycle)
                break
        else:
            # no empty position found
            pass

            # After resolving all the cycles, the board should be solvable with only naive movements
    advance_robots_with_clear_path()

    return moves


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Robot Grid Movement")
    parser.add_argument("--scene", type=str,
                        default="_RANDOM_",  help="Path to scene. To check the format, use --scene-format."
                        "If not provided, a random one will generated"
                        " and dimensions arguments will be required")
    parser.add_argument(
        "-n", type=int, help="Length of the grid. (only when random scene is generated)")
    parser.add_argument(
        "-m", type=int, help="Width of the grid. (only when random scene is generated)")
    parser.add_argument(
        "-k", type=int, help="Number of robbot. (only when random scene is generated)")
    parser.add_argument("--scene-format", action="store_true",
                        help="If provided, will print the scene file format")
    args = parser.parse_args()

    if args.scene_format:
        print_scene_file_format()
        sys.exit(0)

    if args.scene != "_RANDOM_":
        scene = parse_scene_from_file(args.scene)
    else:
        if args.n is None or args.n is None or args.n is None:
            parser.error(
                "n,m,k arguments are required to generate a random scene")
            sys.exit(1)
        scene = generate_random_scene(args.n, args.m, args.k)

    solve_scene(scene)
