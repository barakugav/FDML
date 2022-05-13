#!/usr/bin/env python3

import sys
import math
import random
import time
import threading
import RGM
import RGM_solver
import RGM_gui_ui
from PyQt5.QtCore import Qt, QVariantAnimation, QPointF, pyqtSignal, QObject, QTimer
from PyQt5.QtWidgets import (QApplication, QMainWindow, QGraphicsScene, QGraphicsItem,
                             QGraphicsRectItem, QGraphicsEllipseItem, QDialog, QFileDialog, QMessageBox)
from PyQt5.QtGui import QColor, QBrush, QPen


class RobotItem(QGraphicsRectItem):
    ANIMATION_DURATION = 500

    def __init__(self, cell_size):
        super().__init__(0, 0, cell_size, cell_size)
        self.setFlag(QGraphicsItem.ItemIsMovable, True)

        self.cell_size = cell_size
        self.color = QColor(*random.choices(range(256), k=3))
        self.setBrush(QBrush(self.color))

        self._animation = QVariantAnimation(
            duration=RobotItem.ANIMATION_DURATION)
        self._animation.valueChanged.connect(self.setPos)

    def init_pos(self, x, y):
        self.setPos(x * self.cell_size, y * self.cell_size)

    def move_robot(self, x, y):
        self._animation.setStartValue(self.pos())
        self._animation.setEndValue(
            QPointF(x * self.cell_size, y * self.cell_size))
        self._animation.start()


class TargetItem(QGraphicsEllipseItem):
    def __init__(self, cell_size, color):
        super().__init__(0, 0, cell_size, cell_size)
        self.cell_size = cell_size
        self.color = color
        self.setBrush(
            QBrush(QColor(color.red(), color.green(), color.blue(), 100)))
        self.setPen(QPen(self.color))

    def init_pos(self, x, y):
        self.setPos(x * self.cell_size, y * self.cell_size)


class GuiHooks(QObject):
    signal_robot_move = pyqtSignal(list)
    signal_cycle_identify = pyqtSignal(list)

    CYCLE_IDENTIFY_DURATION = 2000

    def robot_move(self, robot_id, old_pos, new_pos):
        self.signal_robot_move.emit([robot_id, old_pos, new_pos])
        time.sleep(RobotItem.ANIMATION_DURATION / 1000)

    def cycle_identify(self, cycle):
        self.signal_cycle_identify.emit(cycle)
        time.sleep(GuiHooks.CYCLE_IDENTIFY_DURATION / 1000)


class PopoutErrHandler(QObject):
    signal_popout_err = pyqtSignal(str)

    def push(self, msg):
        self.signal_popout_err.emit(msg)

class RGMGui:
    def __init__(self):
        self.scene_desc = None
        self.scene = None

        self.ui = RGM_gui_ui.Ui_MainWindow()
        self.displayed_robots = {}
        self.displayed_targets = {}

        self.popup_err = PopoutErrHandler()
        self.popup_err.signal_popout_err.connect(self._popout_err_impl)

        self._solver_thread = None
        self.gui_hooks = GuiHooks()
        self.gui_hooks.signal_robot_move.connect(self._display_robot_move)
        self.gui_hooks.signal_cycle_identify.connect(self._display_cycle)
        self.cell_size = 50

    def _setup_scene_display(self):
        self.scene_display = QGraphicsScene(self.ui.scene_view)
        self.ui.scene_view.setScene(self.scene_display)

    def _setup_actions(self):
        def action_open_func():
            dialog = QFileDialog(self.win)
            dialog.setWindowTitle("Open scene file")
            dialog.setNameFilter("(*.json)")
            dialog.setFileMode(QFileDialog.ExistingFile)
            dialog_ret = dialog.exec_()
            if dialog_ret != QDialog.Accepted:
                return  # abort
            filenames = dialog.selectedFiles()
            if len(filenames) != 1:
                self._popout_err("only one file can be loaded")
                return
            try:
                scene_desc = RGM.parse_scene_from_file(filenames[0])
            except Exception as e:
                self._popout_err("Failed to load scene", str(e))
                return
            self._load_scene(scene_desc)
        self.ui.action_open.triggered.connect(action_open_func)

        self.ui.solve_button.clicked.connect(self._solve_scene)

        self.ui.reset_button.clicked.connect(self._reset_scene)

    def _reset_scene(self):
        if self._solver_thread is not None:
            self._popout_err("last solving process was not finished yet")
            return
        if self.scene_desc is None:
            self._popout_err("Scene wan't loaded yet")
            return
        self._load_scene(self.scene_desc)

    def _load_scene(self, scene_description):
        if self._solver_thread is not None:
            self._popout_err("last solving process was not finished yet")
            return
        self._clear()

        self.scene_desc = scene_description
        self.scene = RGM_solver.Scene(scene_description)

        for robot in self.scene.robots:
            robot_item = RobotItem(self.cell_size)
            robot_item.init_pos(robot.pos.x, robot.pos.y)
            self.displayed_robots[robot.id] = robot_item

            target_item = TargetItem(self.cell_size, robot_item.color)
            target_item.init_pos(robot.goal.x, robot.goal.y)
            self.displayed_targets[robot.id] = target_item

        for target_item in self.displayed_targets.values():
            self.scene_display.addItem(target_item)
        for robot_item in self.displayed_robots.values():
            self.scene_display.addItem(robot_item)

    def _solve_scene(self):
        if self._solver_thread is not None:
            self._popout_err("last solving process was not finished yet")
            return
        if self.scene_desc is None:
            self._popout_err("Scene wan't loaded yet")
            return

        def thread_target():
            success, _moves, failure_reason = RGM_solver.solve_scene(
                self.scene, self.gui_hooks)
            if success:
                pass
            else:
                failed_cycle, failure_msg = failure_reason[0], failure_reason[1]
                msg = "Failed to solve scene:\n" + failure_msg + "\n" + str(failed_cycle)
                self._popout_err(msg)
            self._solver_thread = None

        self._solver_thread = threading.Thread(target=thread_target)
        self._solver_thread.start()

    def _clear(self):
        if self._solver_thread is not None:
            self._popout_err("last solving process was not finished yet")
            return
        self.scene = None
        for robot_item in self.displayed_robots.values():
            self.scene_display.removeItem(robot_item)
        for target_item in self.displayed_targets.values():
            self.scene_display.removeItem(target_item)

    def _display_robot_move(self, args):
        robot_id, _old_pos, new_pos = args[0], args[1], args[2]
        robot_item = self.displayed_robots[robot_id]
        robot_item.move_robot(new_pos.x, new_pos.y)

    def _display_cycle(self, cycle):
        line_width = math.ceil(self.cell_size / 10)
        displayed_lines = []
        for i in range(len(cycle)):
            r1, r2 = cycle[i], cycle[(i + 1) % len(cycle)]
            p1, p2 = r1.pos, r2.pos
            legnth = self.cell_size * RGM.Position.distance(p1, p2)
            if r1.is_column_robot():
                line = QGraphicsRectItem(0, 0, line_width, legnth + line_width)
            else:  # row robot
                line = QGraphicsRectItem(0, 0, legnth + line_width, line_width)
            x_pos = (min(p1.x, p2.x) + 0.5) * self.cell_size - line_width/2
            y_pos = (min(p1.y, p2.y) + 0.5) * self.cell_size - line_width/2
            line.setPos(x_pos, y_pos)
            line.setBrush(QBrush(QColor(255, 0, 0)))
            line.setPen(QPen(Qt.transparent))
            self.scene_display.addItem(line)
            displayed_lines.append(line)

        def clear_lines():
            for line in displayed_lines:
                self.scene_display.removeItem(line)
        timer = QTimer(self.scene_display)
        timer.timeout.connect(clear_lines)
        timer.setSingleShot(True)
        timer.start(GuiHooks.CYCLE_IDENTIFY_DURATION)

    def _popout_err(self, msg):
        self.popup_err.push(msg)

    def _popout_err_impl(self, msg):
        print(msg) # log the error
        dialog = QMessageBox()
        dialog.setWindowTitle("Error")
        dialog.setText(msg)
        dialog.setIcon(QMessageBox.Critical)
        dialog.exec_()

    def run(self, q_app_args=[sys.argv[0]]):
        app = QApplication(q_app_args)
        self.win = QMainWindow()
        self.ui.setupUi(self.win)

        self._setup_scene_display()
        self._setup_actions()

        self.win.show()
        sys.exit(app.exec_())


if __name__ == "__main__":
    gui = RGMGui()
    gui.run(sys.argv)
