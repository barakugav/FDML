#!/usr/bin/env python3

from sortedcontainers import SortedList
import RGM
from RGM import Direction, Position
import argparse
import json


# Robot Grid Movement (RGM) solver


def get_paths_intersection(p1, p2):
    if p1[0].x == p1[1].x:
        assert p2[0].y == p2[1].y
        return Position(p1[0].x, p2[0].y)
    else:
        assert p1[0].y == p1[1].y
        assert p2[0].x == p2[1].x
        return Position(p1[0].y, p2[0].x)


def is_paths_intersect(p1, p2):
    def order_range(a, b): return (a, b) if a <= b else (b, a)
    x11, x12 = order_range(p1[0].x, p1[1].x)
    y11, y12 = order_range(p1[0].y, p1[1].y)
    x21, x22 = order_range(p2[0].x, p2[1].x)
    y21, y22 = order_range(p2[0].y, p2[1].y)
    return x11 <= x22 and x12 >= x21 and y11 <= y22 and y12 >= y21


class Robot:
    def __init__(self, id, start, goal):
        self.id = id
        self.start = start
        self.goal = goal
        self.pos = start

        self.succr = [None] * len(Direction)

    def is_column_robot(self):
        return self.start.x == self.goal.x

    def is_reached_goal(self):
        return self.pos == self.goal

    def get_goal_direction(self):
        if self.is_reached_goal():
            return None
        if self.is_column_robot():
            return Direction.Yp if self.pos.y < self.goal.y else Direction.Yn
        else:
            return Direction.Xp if self.pos.x < self.goal.x else Direction.Xn

    def __repr__(self):
        return "{%s, %s, %s}" % (self.pos, self.start, self.goal)


class Scene:
    def __init__(self, description):
        self.width = description.width
        self.height = description.height
        self.robots = [Robot(id, r.start, r.goal)
                       for id, r in enumerate(description.paths)]


def _print_scene(scene, print_successors=False):
    robots = sorted(
        scene.robots, key=lambda robot: robot.pos.x + robot.pos.y * scene.width)

    def pos_next(pos):
        return Position(pos.x + 1, pos.y) if pos.x < scene.width - 1 else Position(0, pos.y+1)

    def print_empty(from_pos, to_pos):
        while from_pos != to_pos:
            print("|  ", end="")
            if from_pos.x == scene.width - 1:
                print("|")
                print("-" * (scene.width * 3 + 1))
            from_pos = pos_next(from_pos)

    printed_until = Position(0, 0)
    print("-" * (scene.width * 3 + 1))
    for robot in robots:
        print_empty(printed_until, robot.pos)
        print("|{:02}".format(robot.id), end="")
        if robot.pos.x == scene.width - 1:
            print("|")
            print("-" * (scene.width * 3 + 1))
        printed_until = pos_next(robot.pos)
    print_empty(printed_until, Position(0, scene.height))

    if print_successors:
        print(" ID: X+ X- Y+ Y-")
        for robot in robots:
            ids = [
                "--" if r is None else "{:02}".format(r.id) for r in robot.succr]
            print("R{:02}: {} {} {} {} {}".format(robot.id, *ids, robot))


def solve_scene(scene, gui_hooks=None):
    if gui_hooks is None:
        class Object(object):
            pass
        gui_hooks = Object()
        empty_func = lambda *_: None
        gui_hooks.robot_move = empty_func
        gui_hooks.cycle_identify = empty_func

    # TODO validate scene
    for robot in scene.robots:
        for p in [robot.start, robot.goal, robot.pos]:
            if not Position.is_in_bounds([0, scene.width], [0, scene.height], p.x, p.y):
                raise ValueError(
                    "Invalid scene! robot is out of bounds", p, (scene.width, scene.height))

    moves = []

    def move_robot(robot, new_pos):
        assert robot.pos != new_pos
        if robot.is_column_robot():
            assert robot.pos.x == new_pos.x
        else:
            assert robot.pos.y == new_pos.y
        old_pos = robot.pos
        robot.pos = new_pos
        moves.append((robot.id, old_pos, new_pos))
        gui_hooks.robot_move(robot.id, old_pos, new_pos)

    def calculate_successors():
        for robot in scene.robots:
            robot.succr = [None] * len(Direction)
        # Perform sweep to calculate successors x+,x-,y+,y- of all robots
        # Running time is O(k log k)
        visible_robots = SortedList(key=lambda robot: robot.pos.y)
        prev_col_robot = None
        for robot in sorted(scene.robots, key=lambda r: r.pos):
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

    # Calculate successors from start position
    # During the algorithm, this method will be called again to calculate the successors,
    # as it's harder to maintain these values than calculating them again
    calculate_successors()

    def get_blocking_robot(robot):
        if robot.is_reached_goal():
            return None
        blocking_r = robot.succr[robot.get_goal_direction()]
        if blocking_r is None:
            return None
        block_dis = Position.distance(robot.pos, blocking_r.pos)
        goal_dis = Position.distance(robot.pos, robot.goal)
        return blocking_r if block_dis <= goal_dis else None

    def has_clear_path(robot):
        return get_blocking_robot(robot) is None

    def update_near_successors_on_leave(robot):
        changed_robots = []
        if robot.is_column_robot():
            if robot.succr[Direction.Xp] is not None:
                robot.succr[Direction.Xp].succr[Direction.Xn] = robot.succr[Direction.Xn]
                changed_robots.append(robot.succr[Direction.Xp])
            if robot.succr[Direction.Xn] is not None:
                robot.succr[Direction.Xn].succr[Direction.Xp] = robot.succr[Direction.Xp]
                changed_robots.append(robot.succr[Direction.Xn])
        else:  # row robot
            if robot.succr[Direction.Yp] is not None:
                robot.succr[Direction.Yp].succr[Direction.Yn] = robot.succr[Direction.Yn]
                changed_robots.append(robot.succr[Direction.Yp])
            if robot.succr[Direction.Yn] is not None:
                robot.succr[Direction.Yn].succr[Direction.Yp] = robot.succr[Direction.Yp]
                changed_robots.append(robot.succr[Direction.Yn])
        return changed_robots

    # After moving one robot, successors should be check if their path was cleared
    # Running time is O(k)
    def advance_robots_with_clear_path():
        for robot in scene.robots:
            queue = [robot]
            while len(queue) > 0:
                robot = queue.pop()
                if not has_clear_path(robot):
                    continue
                if robot.is_reached_goal():
                    continue

                # update near robots' successors attributes
                queue += update_near_successors_on_leave(robot)

                # Actual move
                move_robot(robot, robot.goal)

                # Clear successors
                robot.succr = [None] * len(Direction)

    # Start by moving all robots with clear path to their target
    advance_robots_with_clear_path()
    calculate_successors()

    # Calculate all cycles
    cycles = []
    visited = [False] * len(scene.robots)
    for robot in scene.robots:
        if robot.is_reached_goal() or visited[robot.id]:
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

        def get_pos(self):
            p1 = [self.column_r.pos, self.column_r.goal]
            p2 = [self.row_r.pos, self.row_r.goal]
            return get_paths_intersection(p1, p2)

    # Solve each cycle independently
    for cycle in cycles:
        gui_hooks.cycle_identify(cycle)

        assert len(cycle) >= 4

        def prev_cycle_idx(idx):
            return (idx - 1) % len(cycle)

        def next_cycle_idx(idx):
            return (idx + 1) % len(cycle)

        # Calculate all intersections
        intersections = [[] for _ in range(len(scene.robots))]
        for idx, robot in enumerate(cycle):
            if not robot.is_column_robot():
                continue  # arbitrary choose to iterate only on column robots
            assert not robot.is_reached_goal()
            prev_robot = cycle[prev_cycle_idx(idx)]
            next_robot = cycle[next_cycle_idx(idx)]
            robot_path = [robot.pos, next_robot.pos]

            for idx_other, robot_other in enumerate(cycle):
                if robot_other in [prev_robot, robot, next_robot]:
                    continue
                next_robot_other = cycle[next_cycle_idx(idx_other)]
                robot_other_path = [robot_other.pos, next_robot_other.pos]
                if is_paths_intersect(robot_path, robot_other_path):
                    intersection = Intersection(robot, robot_other)
                    intersections[robot.id].append(intersection)
                    intersections[robot_other.id].append(intersection)

        # Search for an empty position which is not an intersection
        empty_pos = None
        for idx, robot in enumerate(cycle):
            assert not robot.is_reached_goal()
            next_robot = cycle[next_cycle_idx(idx)]
            robot_direction = robot.get_goal_direction()

            intersections[robot.id].sort(
                key=(lambda v: v.row_r.pos.y if robot.is_column_robot()
                     else v.column_r.pos.x),
                reverse=not robot_direction.is_positive())

            maybe_empty_pos = robot.pos.add(robot_direction)
            for intersection in intersections[robot.id]:
                if maybe_empty_pos != intersection.get_pos():
                    break
                maybe_empty_pos = maybe_empty_pos.add(robot_direction)
            if maybe_empty_pos != next_robot.pos:
                empty_pos = maybe_empty_pos
                break

        if empty_pos is not None:
            # Found empty position. Advance a robot to the empty position, and all other robots to their
            # cycle's target position, solving the cycle.
            prev_pos = robot.pos
            move_robot(robot, empty_pos)

            idx = prev_cycle_idx(idx)
            while True:
                r = cycle[idx]
                prev_pos_new = r.pos
                move_robot(r, prev_pos)
                prev_pos = prev_pos_new
                if r == robot:
                    break
                idx = prev_cycle_idx(idx)
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
                    self.target_dir = None
                    self.robots = []

                def addRobot(self, robot):
                    self.robots.append(robot)

            # Find first robot with intersections
            first_robot_idx_with_inters = None
            for idx, robot in enumerate(cycle):
                if len(intersections[robot.id]) > 0:
                    first_robot_idx_with_inters = idx
                    break
            if first_robot_idx_with_inters is None:
                return (False, None, (cycle, "Cycle with no gaps! No solution"))

            # Build intersection graph
            current_edge = None
            idx = first_robot_idx_with_inters
            edges_num = 0
            while True:
                robot = cycle[idx]
                direction = robot.get_goal_direction()

                for inter in intersections[robot.id]:
                    if inter.inter_vertex is None:
                        inter.inter_vertex = IntersectionVertex()
                    vertex = inter.inter_vertex

                    if current_edge is not None:
                        current_edge.target_dir = direction.opposite()
                        vertex.edge[current_edge.target_dir] = current_edge
                        current_edge.target = vertex
                    current_edge = Edge()
                    current_edge.source_dir = direction
                    vertex.edge[current_edge.source_dir] = current_edge
                    current_edge.source = vertex
                    edges_num += 1

                idx = next_cycle_idx(idx)
                robot = cycle[idx]
                current_edge.addRobot(robot)
                if idx == first_robot_idx_with_inters:
                    robot = cycle[idx]
                    direction = robot.get_goal_direction()
                    vertex = intersections[robot.id][0].inter_vertex
                    current_edge.target_dir = direction.opposite()
                    vertex.edge[current_edge.target_dir] = current_edge
                    current_edge.target = vertex
                    init_edge = current_edge  # arbitrary edge, used to start a traversal
                    break

            # Contract all loops
            steps_without_contrtaction = 0
            steps_without_contrtaction_limit = edges_num
            edge = init_edge
            while True:
                prev_edge = edge.source.edge[edge.source_dir.opposite()]
                next_edge = edge.target.edge[edge.target_dir.opposite()]
                if edge.source == edge.target:
                    # Found self loop, contract
                    if prev_edge == next_edge:
                        return (False, None, (cycle, "Contracted intersection graph until a single loop achieved! No solution"))
                    assert prev_edge.source != prev_edge.target
                    assert next_edge.source != next_edge.target

                    new_edge = Edge()
                    new_edge.source = prev_edge.source
                    new_edge.source_dir = prev_edge.source_dir
                    new_edge.source.edge[new_edge.source_dir] = new_edge
                    new_edge.target = next_edge.target
                    new_edge.target_dir = next_edge.target_dir
                    new_edge.target.edge[new_edge.target_dir] = new_edge
                    new_edge.robots = prev_edge.robots + edge.robots + next_edge.robots

                    edge = new_edge
                    steps_without_contrtaction = 0
                else:
                    edge = next_edge
                    steps_without_contrtaction += 1
                    if steps_without_contrtaction >= steps_without_contrtaction_limit:
                        # No more contrtactions
                        break

            # Expand graph and calculate all movement
            # 1. Start with an arbitrary edge with >0 robots
            # 2. Advance all robots in the edge one step
            # 3. Move to next edge, and go to 2.

            # Find an edge with >0 robots
            while True:
                if len(edge.robots) > 0:
                    break
                edge = edge.target.edge[edge.target_dir.opposite()]
            starting_edge = edge

            prev_last_robot = None
            while True:
                # Advance all robots in edge by one tile
                if len(edge.robots) > 0:
                    prev_pos = None
                    for robot in reversed(edge.robots):
                        curr_pos = robot.pos
                        if prev_pos is None:
                            move_robot(robot, robot.pos.add(
                                robot.get_goal_direction()))
                        else:
                            move_robot(robot, prev_pos)
                        prev_pos = curr_pos
                    prev_last_robot_new = edge.robots[len(edge.robots) - 1]
                else:
                    prev_last_robot_new = prev_last_robot

                if prev_last_robot is not None:
                    move_robot(prev_last_robot, prev_last_robot.pos.add(
                        prev_last_robot.get_goal_direction()))
                prev_last_robot = prev_last_robot_new

                edge = edge.target.edge[edge.target_dir.opposite()]
                if edge == starting_edge:
                    move_robot(prev_last_robot, prev_last_robot.pos.add(
                        prev_last_robot.get_goal_direction()))
                    break
    calculate_successors()

    # After resolving all the cycles, the scene should be solvable with only naive movements
    advance_robots_with_clear_path()

    for robot in scene.robots:
        if not robot.is_reached_goal():
            print("Robot", robot.id, "failed to reach it's goal position.")
            print("Either a scene that doesn't fit the requirements or a bug.")
            return (False, None, ([], "invalid scene or bug"))

    return (True, moves, None)


def main(scene_filename, out_filename=None):
    scene_desc = RGM.parse_scene_from_file(scene_filename)
    success, moves, failure_reason = solve_scene(Scene(scene_desc))
    if not success:
        failed_cycle, failure_msg = failure_reason[0], failure_reason[1]
        print("Failed to solve scene:")
        print(failure_msg)
        print(failed_cycle)
        return
    if out_filename == None:
        print("Solution moves:")
        for move in moves:
            start_pos, end_pos = move[1], move[2]
            print(start_pos, end_pos)
    else:
        def position_to_jsonobj(p):
            return [p.x, p.y]

        def move_to_jsonobj(m):
            return [m[0], position_to_jsonobj(m[1]), position_to_jsonobj(m[2])]
        json_string = json.dumps([move_to_jsonobj(move) for move in moves])
        with open(out_filename, "w") as outfile:
            outfile.write(json_string)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Robot Grid Movement solver")
    parser.add_argument("--scene", type=str,
                        help="Path to scene description to solve")
    parser.add_argument("--out", type=str, default="_STDOUT_",
                        help="Path to output file")
    parser.add_argument("--scene-format", action="store_true",
                        help="If provided, will print the scene file format")
    args = parser.parse_args()

    if args.scene_format:
        print(""""width": 10,
"height": 10,
"paths": [
    [[0,0], [0,8]],
    [{start_pos}, {goal_pos}],
    ...
]""")
    elif args.scene:
        main(args.scene, args.out if args.out != "_STDOUT_" else None)
    else:
        parser.print_help()
