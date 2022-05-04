#!/usr/bin/env python3

import sys
import random
from enum import Enum
from sortedcontainers import SortedList
import argparse


class Direction(Enum):
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
        if direction == Direction.yp:
            return Position(self.x, self.y + 1)
        if direction == Direction.yn:
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

        self.succr = [None] * len(Direction)

    def get_goal_direction(self):
        if self.pos == self.goal:
            return None
        if self.pos.x == self.goal.x:
            return Direction.Yp if self.pos.y < self.goal.y else Direction.Yn
        else:
            return Direction.Xp if self.pos.x < self.goal.x else Direction.Xn

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
            ids = [
                "--" if r is None else "{:02}".format(r.id) for r in robot.succr]
            print("R{:02}: {} {} {} {} {}".format(robot.id, *ids, robot))


def solve_scene(scene):
    # TODO validate scene

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
            prev_col_robot.succr[Direction.Yp] = robot
            robot.succr[Direction.Yn] = prev_col_robot
        prev_col_robot = robot

        # Check if there is some robot exactly to the left (negative x, xn) of the current robot
        prev_row_robot_idx = visible_robots.bisect_left(robot)
        prev_row_robot = visible_robots[prev_row_robot_idx] if prev_row_robot_idx < len(
            visible_robots) else None
        prev_row_robot = prev_row_robot if prev_row_robot is not None and prev_row_robot.pos.y == robot.pos.y else None
        if prev_row_robot is not None:
            assert robot.pos.x > prev_row_robot.pos.x
            prev_row_robot.succr[Direction.Xp] = robot
            robot.succr[Direction.Xn] = prev_row_robot
            visible_robots.remove(prev_row_robot)
        visible_robots.add(robot)
    visible_robots.clear()

    print_board(board, print_successors=True)  # TODO remove

    def get_blocking_robot(robot):
        if robot.pos == robot.goal:
            return None
        blocking_r = robot.succr[robot.get_goal_direction()]
        if blocking_r is None:
            return None
        block_dis = Position.distance(robot.pos, robot.goal)
        goal_dis = Position.distance(robot.pos, blocking_r.pos)
        return blocking_r if block_dis <= goal_dis else None

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
                    if robot.succr[Direction.Xp] is not None:
                        robot.succr[Direction.Xp].succr[Direction.Xn] = robot.succr[Direction.Xn]
                        queue.append(robot.succr[Direction.Xp])
                    if robot.succr[Direction.Xn] is not None:
                        robot.succr[Direction.Xn].succr[Direction.Xp] = robot.succr[Direction.Xp]
                        queue.append(robot.succr[Direction.Xn])
                else:
                    # row robot
                    if robot.succr[Direction.Yp] is not None:
                        robot.succr[Direction.Yp].succr[Direction.Yn] = robot.succr[Direction.Yn]
                        queue.append(robot.succr[Direction.Yp])
                    if robot.succr[Direction.Yn] is not None:
                        robot.succr[Direction.Yn].succr[Direction.Yp] = robot.succr[Direction.Yp]
                        queue.append(robot.succr[Direction.Yn])

                # Actual move
                move_robot(robot, robot.goal)

                # Clear successors
                robot.succr = [None] * len(Direction)

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

    class Intersection:
        def __init__(self, column_robot, row_robot):
            self.column_r = column_robot
            self.row_r = row_robot
            self.inter_vertex = None

    # Solve each cycle independently
    for cycle in cycles:
        assert len(cycle) >= 4

        # Calculate all intersection
        intersections = [[] for _ in range(len(board.robots))]
        for idx, robot in enumerate(cycle):
            if robot.pos.x != robot.goal.x:
                continue  # arbitrary choose to iterate only on column robots
            assert robot.pos != robot.goal
            prev_robot = cycle[(idx - 1) % len(cycle)]
            next_robot = cycle[(idx + 1) % len(cycle)]
            robot_path = [robot.pos, next_robot.pos]

            for idx_other, robot_other in enumerate(cycle):
                if robot_other in [prev_robot, robot, next_robot]:
                    continue
                next_robot_other = cycle[(idx_other + 1) % len(cycle)]
                robot_other_path = [robot_other.pos, next_robot_other.pos]
                if is_paths_intersect(robot_path, robot_other_path):
                    intersection = Intersection(robot, robot_other_path)
                    intersections[robot.id].append(intersection)
                    intersections[robot_other.id].append(intersection)

        # Search for an empty position which is not an intersection
        empty_pos = None
        for idx, robot in enumerate(cycle):
            assert robot.pos != robot.goal
            next_robot = cycle[(idx + 1) % len(cycle)]
            robot_path = [robot.pos, next_robot.pos]
            robot_direction = robot.get_goal_direction()

            intersections[robot.id].sort(
                key=(lambda v: v.row_r.pos.y if robot.pos.x ==
                     robot.goal.x else v.column_r.pos.x),
                reverse=not robot_direction.is_positive())

            maybe_empty_pos = robot.pos.add(robot_direction)
            for robot_other in intersections[robot.id]:
                if maybe_empty_pos != robot_other.pos:
                    break
                maybe_empty_pos.add(robot_direction)
            if maybe_empty_pos != next_robot.pos:
                empty_pos = maybe_empty_pos
                break

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
        else:
            # No empty position found. Build the intersection graph and try to solve it

            class IntersectionVertex:
                def __init__(self):
                    self.edge = [None] * len(Direction)

            class Edge:
                def __init__(self):
                    self.source = None
                    self.target = None
                    self.source_dir = None
                    self.targer_dir = None

                    self._robots = []
                    self._merge = None

                def add(self, robot):
                    self._robots.append(robot)

                @staticmethod
                def from_merge(e1, e2_self, e3, v):
                    edge = Edge()

                    edge.source = e1.source
                    edge.source_dir = e1.source_dir
                    edge.source.edge[e1.source_dir] = edge

                    edge.target = e1.target
                    edge.target_dir = e3.target_dir
                    edge.target.edge[e3.target_dir] = edge

                    edge._merge = (e1, e2_self, e3, v)

            # Find first robot with intersections
            first_robot_with_inters = None
            for idx, robot in enumerate(cycle):
                if len(intersections[robot.id]) > 0:
                    first_robot_idx_with_inters = idx
                    break
            if first_robot_idx_with_inters is None:
                raise ValueError("Cycle with no gaps! No solution")

            # Build intersection graph
            current_edge = None
            idx = first_robot_idx_with_inters
            while True:
                robot = cycle[idx]
                direction = robot.get_goal_direction()

                for inter in intersections[robot.id]:
                    if inter.inter_vertex is None:
                        inter.inter_vertex = IntersectionVertex()
                    vertex = inter.inter_vertex

                    if current_edge is not None:
                        current_edge.targer_dir = direction.opposite()
                        vertex.edge[current_edge.targer_dir] = current_edge
                        current_edge.target = vertex
                    current_edge = Edge()
                    current_edge.source_dir = direction
                    vertex.edge[current_edge.source_dir] = current_edge
                    current_edge.source = vertex

                    init_edge = current_edge # arbitrary edge, used to start a traversal

                idx = (idx + 1) % len(cycle)
                current_edge.add(robot = cycle[idx])
                if idx == first_robot_idx_with_inters:
                    break

            # Contract all loops
            edge = init_edge
            while True:
                if edge.target is None:
                    raise ValueError("Contracted cycle until a single loop! No solution")
                next_edge = edge.target.edge[edge.targer_dir.opposite()]
                if next_edge.source == next_edge.target:
                    # Found self loop, contract
                    next_next_edge = next_edge.target.edge[next_edge.targer_dir.opposite()]
                    # TODO merge

                else:
                    edge = next_edge

            # Expand graph and calculate all movement
            # TODO


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
