#!/usr/bin/env python3

import sys
import os
import json
from PyQt5 import QtCore, QtGui, QtWidgets
from PyQt5.QtCore import pyqtSignal
from PyQt5.QtWidgets import QFileDialog
import ctypes
from geometrygui.gui import GUI, MainWindowPlus

IMG_DIR = os.path.abspath(os.path.join(
    os.path.dirname(os.path.realpath(__file__)), "../img"))


class MainWindowSceneDesigner(MainWindowPlus):
    signal_ctrl_z = pyqtSignal()
    signal_delete = pyqtSignal()
    signal_esc = pyqtSignal()
    signal_drop = pyqtSignal(str)

    def __init__(self, gui):
        super().__init__(gui)

    # Adjust zoom level/scale on +/- key press
    def keyPressEvent(self, event):
        if event.key() == QtCore.Qt.Key_Plus:
            self.gui.zoom /= 0.9
        if event.key() == QtCore.Qt.Key_Minus:
            self.gui.zoom *= 0.9
        if event.modifiers() & QtCore.Qt.ControlModifier and event.key() == QtCore.Qt.Key_Z:
            self.signal_ctrl_z.emit()
        if event.key() == QtCore.Qt.Key_Escape:
            self.signal_esc.emit()
        if event.key() == QtCore.Qt.Key_Delete:
            self.signal_delete.emit()
        self.gui.redraw()


class SceneDesignerGUIComponent(GUI):
    def __init__(self):
        super().__init__()
        self.textEdits = []
        self.zoom = 50.0
        self.set_program_name("Robot Localization")
        self.mainWindow.setWindowIcon(
            QtGui.QIcon(os.path.join(IMG_DIR, "icon.png")))
        self.redraw()

    def setup_ui(self):
        self.mainWindow = MainWindowSceneDesigner(self)
        main_window = self.mainWindow

        main_window.setStyleSheet("QMainWindow { background-color : rgb(54, 57, 63); color : rgb(220, 221, 222); }\n"
                                  "#centralwidget { background-color : rgb(54, 57, 63); color : rgb(220, 221, 222); }\n"
                                  "QLabel { background-color : rgb(54, 57, 63); color : rgb(220, 221, 222); }")
        self.centralwidget = QtWidgets.QWidget(main_window)
        self.centralwidget.setObjectName("centralwidget")
        self.gridLayout = QtWidgets.QGridLayout(self.centralwidget)
        self.gridLayout.setObjectName("gridLayout")
        self.graphics_view = QtWidgets.QGraphicsView(self.centralwidget)
        self.graphics_view.setEnabled(True)
        sizePolicy = QtWidgets.QSizePolicy(
            QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Expanding)
        sizePolicy.setHorizontalStretch(1)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(
            self.graphics_view.sizePolicy().hasHeightForWidth())
        self.graphics_view.setSizePolicy(sizePolicy)
        self.graphics_view.setObjectName("graphics_view")
        self.gridLayout.addWidget(self.graphics_view, 3, 1, 1, 1)
        self.gridLayout_0 = QtWidgets.QGridLayout()
        self.gridLayout_0.setObjectName("gridLayout_0")
        spacerItem = QtWidgets.QSpacerItem(300, 20, QtWidgets.QSizePolicy.MinimumExpanding,
                                           QtWidgets.QSizePolicy.Minimum)
        self.gridLayout_0.addItem(spacerItem, 33, 0, 1, 1)

        font = QtGui.QFont()
        font.setPointSize(12)

        # Scene load
        self.scene_load_label = QtWidgets.QLabel(self.centralwidget)
        self.scene_load_label.setFont(font)
        self.scene_load_label.setObjectName("scene_load_label")
        self.gridLayout_0.addWidget(self.scene_load_label, 1, 0, 1, 1)
        self.scene_load_intxt = QtWidgets.QLineEdit(self.centralwidget)
        self.scene_load_intxt.setFont(font)
        self.scene_load_intxt.setObjectName("scene_load_intxt")
        self.gridLayout_0.addWidget(self.scene_load_intxt, 2, 0, 1, 1)
        self.scene_load_dialog_button = QtWidgets.QToolButton(
            self.centralwidget)
        self.scene_load_dialog_button.setFont(font)
        self.scene_load_dialog_button.setObjectName("scene_load_dialog_button")
        self.gridLayout_0.addWidget(self.scene_load_dialog_button, 2, 1, 1, 1)
        self.scene_load_button = QtWidgets.QPushButton(self.centralwidget)
        self.scene_load_button.setFont(font)
        self.scene_load_button.setObjectName("scene_load_button")
        self.gridLayout_0.addWidget(self.scene_load_button, 3, 0, 1, 1)

        # Scene save
        self.scene_save_label = QtWidgets.QLabel(self.centralwidget)
        self.scene_save_label.setFont(font)
        self.scene_save_label.setObjectName("scene_save_label")
        self.gridLayout_0.addWidget(self.scene_save_label, 4, 0, 1, 1)
        self.scene_save_intxt = QtWidgets.QLineEdit(self.centralwidget)
        self.scene_save_intxt.setFont(font)
        self.scene_save_intxt.setObjectName("scene_save_intxt")
        self.gridLayout_0.addWidget(self.scene_save_intxt, 5, 0, 1, 1)
        self.scene_save_dialog_button = QtWidgets.QToolButton(
            self.centralwidget)
        self.scene_save_dialog_button.setFont(font)
        self.scene_save_dialog_button.setObjectName("scene_save_dialog_button")
        self.gridLayout_0.addWidget(self.scene_save_dialog_button, 5, 1, 1, 1)
        self.scene_save_button = QtWidgets.QPushButton(self.centralwidget)
        self.scene_save_button.setFont(font)
        self.scene_save_button.setObjectName("scene_save_button")
        self.gridLayout_0.addWidget(self.scene_save_button, 6, 0, 1, 1)

        # Resolution set
        self.resolution_label = QtWidgets.QLabel(self.centralwidget)
        self.resolution_label.setFont(font)
        self.resolution_label.setObjectName("resolution_label")
        self.gridLayout_0.addWidget(self.resolution_label, 7, 0, 1, 1)
        self.resolution_intxt = QtWidgets.QLineEdit(self.centralwidget)
        self.resolution_intxt.setFont(font)
        self.resolution_intxt.setObjectName("resolution_intxt")
        self.gridLayout_0.addWidget(self.resolution_intxt, 8, 0, 1, 1)
        self.resolution_set_button = QtWidgets.QPushButton(self.centralwidget)
        self.resolution_set_button.setFont(font)
        self.resolution_set_button.setObjectName("resolution_set_button")
        self.gridLayout_0.addWidget(self.resolution_set_button, 9, 0, 1, 1)

        # Test TODO and clear
        self.textEdit = QtWidgets.QTextEdit(self.centralwidget)
        self.textEdit.setObjectName("textEdit")
        self.gridLayout_0.addWidget(self.textEdit, 10, 0, 1, 1)
        self.clear_button = QtWidgets.QPushButton(self.centralwidget)
        self.clear_button.setFont(font)
        self.clear_button.setObjectName("clear_button")
        self.gridLayout_0.addWidget(self.clear_button, 11, 0, 1, 1)

        spacerItem1 = QtWidgets.QSpacerItem(
            20, 40, QtWidgets.QSizePolicy.Minimum, QtWidgets.QSizePolicy.Expanding)
        self.gridLayout_0.addItem(spacerItem1, 30, 0, 1, 1)
        self.gridLayout.addLayout(self.gridLayout_0, 3, 0, 1, 1)
        main_window.setCentralWidget(self.centralwidget)
        self.statusbar = QtWidgets.QStatusBar(main_window)
        self.statusbar.setObjectName("statusbar")
        main_window.setStatusBar(self.statusbar)

        QtCore.QMetaObject.connectSlotsByName(main_window)

        self.labels['scene_load'] = self.scene_load_label
        self.lineEdits['scene_load'] = self.scene_load_intxt
        self.pushButtons['scene_load_dialog'] = self.scene_load_dialog_button
        self.pushButtons['scene_load'] = self.scene_load_button

        self.labels['scene_save'] = self.scene_save_label
        self.lineEdits['scene_save'] = self.scene_save_intxt
        self.pushButtons['scene_save_dialog'] = self.scene_save_dialog_button
        self.pushButtons['scene_save'] = self.scene_save_button

        self.labels['resolution'] = self.resolution_label
        self.lineEdits['resolution'] = self.resolution_intxt
        self.pushButtons['resolution_set'] = self.resolution_set_button

        self.pushButtons['clear'] = self.clear_button


def calc_slope_intersection(s):
    if s[0][0] == s[1][0]:
        raise ValueError()
    m = (s[0][1] - s[1][1]) / (s[0][0] - s[1][0])
    b = s[0][1] - m * s[0][0]
    return m, b


def calc_1d_intersection(x1, x2):
    x1_low = min(x1[0], x1[1])
    x1_high = max(x1[0], x1[1])
    x2_low = min(x2[0], x2[1])
    x2_high = max(x2[0], x2[1])
    if x1_low <= x2_low and x2_low <= x1_high:
        return (x1_low + min(x1_high, x2_high)) / 2
    if x1_low <= x2_high and x2_high <= x1_high:
        return (x2_high + min(x1_low, x2_low)) / 2
    if x2_low <= x1_low and x1_low <= x2_high:
        return (x2_low + min(x2_high, x1_high)) / 2
    if x2_low <= x1_high and x1_high <= x2_high:
        return (x1_high + min(x2_low, x1_low)) / 2
    return None


def calc_intersection(s1, s2):
    s1_vertical = s1[0][0] == s1[1][0]
    s2_vertical = s2[0][0] == s2[1][0]
    if s1_vertical and s2_vertical:
        if s1[0][0] != s2[0][0]:
            return None
        inter_y = calc_1d_intersection(
            [s1[0][1], s1[1][1]], [s2[0][1], s2[1][1]])
        return None if inter_y is None else [s1[0][0], inter_y]

    if s2_vertical:
        s1, s2 = s2, s1
        s1_vertical, s2_vertical = s2_vertical, s1_vertical
    m2, b2 = calc_slope_intersection(s2)
    s2x_low = min(s2[0][0], s2[1][0])
    s2x_high = max(s2[0][0], s2[1][0])
    if s1_vertical:
        if s1[0][0] < s2x_low or s1[0][0] > s2x_high:
            return None
        s2y = m2 * s1[0][0] + b2
        s1y_low = min(s1[0][1], s1[1][1])
        s1y_high = max(s1[0][1], s1[1][1])
        if s1y_low <= s2y and s2y <= s1y_high:
            return [s1[0][0], s2y]
        return None

    m1, b1 = calc_slope_intersection(s1)
    if m1 == m2:
        if b1 != b2:
            return None
        inter_x = calc_1d_intersection(
            [s1[0][0], s1[1][0]], [s2[0][0], s2[1][0]])
        return None if inter_x is None else [inter_x, m1 * inter_x + b1]

    s1x_low = min(s1[0][0], s1[1][0])
    s1x_high = max(s1[0][0], s1[1][0])
    inter_x = (b2 - b1) / (m1 - m2)
    if s1x_low <= inter_x and inter_x <= s1x_high and s2x_low <= inter_x and inter_x <= s2x_high:
        return [inter_x, m1 * inter_x + b1]
    return None


def is_simple_polygon(points):
    if len(points) <= 2:
        return True
    segments = []
    for i in range(1, len(points)):
        segments.append([points[i - 1], points[i]])
    segments.append([points[len(points) - 1], points[0]])
    for i in range(0, len(segments)):
        ignore = [
            i - 1 if i != 0 else len(segments) - 1, i, i + 1 if i != len(segments) - 1 else 0]
        for j in range(0, len(segments)):
            inter = calc_intersection(segments[i], segments[j])
            if inter != None and j not in ignore:
                return False
    return True


def open_file_dialog(init_dir=""):
    dlg = QFileDialog()
    dlg.setFileMode(QFileDialog.AnyFile)
    dlg.setDirectory(init_dir)
    if dlg.exec_():
        filenames = dlg.selectedFiles()
        return filenames[0]


POINT_RADIUS = 0.1
GRID_SIZE = 200


class SceneDesignerGUI:
    def __init__(self, args=[]):
        self.resolution = 1.0
        self.polygon_obstacles = []
        self.gui_polygon_obstacles = []
        self.polyline = []
        self.gui_current_polygon_vertices = []
        self.gui_current_polygon_edges = []
        self.grid = []

        self._app = QtWidgets.QApplication(sys.argv)
        self.gui = SceneDesignerGUIComponent()
        self.gui.scene.left_click_signal.connect(self._left_click)
        self.gui.mainWindow.signal_ctrl_z.connect(self._undo)
        self.gui.mainWindow.signal_esc.connect(self._clear_current_polyline)
        self.gui.mainWindow.signal_drop.connect(self._load_scene_button)
        self.gui.set_animation_finished_action(lambda: None)

        self._setup_logic()

        self._redraw_grid(GRID_SIZE)
        self.gui.add_disc(POINT_RADIUS, 0, 0)

    def _setup_logic(self):
        self.gui.set_label('scene_load', 'Existing scene:')
        self.gui.set_button_text('scene_load_dialog', '..')
        self.gui.set_logic('scene_load_dialog', self._open_scene_load_dialog)
        self.gui.set_button_text('scene_load', 'Load Scene')
        self.gui.set_logic('scene_load', self._load_scene_button)

        self.gui.set_label('scene_save', 'Output Path:')
        self.gui.set_button_text('scene_save_dialog', '..')
        self.gui.set_logic('scene_save_dialog', self._open_scene_save_dialog)
        self.gui.set_button_text('scene_save', 'Save Scene')
        self.gui.set_logic('scene_save', self._save_scene_button)

        self.gui.set_label('resolution', 'Resolution')
        self.gui.set_button_text('resolution_set', 'Set Resolution')
        self.gui.set_field('resolution', str(self.resolution))
        self.gui.set_logic('resolution_set', self._set_resolution)

        self.gui.set_button_text('clear', 'Clear scene')
        self.gui.set_logic('clear', self._clear)

    def _display_print(self, *args):
        s = ""
        for a in args:
            s += str(a)
        self.gui.textEdit.setPlainText(s)

    def _clear_current_polyline(self):
        self.polyline = []
        for vertex in self.gui_current_polygon_vertices:
            self.gui.scene.removeItem(vertex.disc)
        self.gui_current_polygon_vertices.clear()
        for edge in self.gui_current_polygon_edges:
            self.gui.scene.removeItem(edge.line)
        self.gui_current_polygon_edges.clear()

    def _submit_polygon_obstacle(self, polygon):
        if not is_simple_polygon(polygon):
            self._display_print("invalid polygon!")
            return False
        self.polygon_obstacles.append(polygon)
        self.gui_polygon_obstacles.append(self.gui.add_polygon(
            polygon, fill_color=QtCore.Qt.transparent, line_color=QtCore.Qt.blue))
        return True

    def _left_click(self, x, y):
        if len(self.polygon_obstacles) > 0:
            self._clear()
        x = self.resolution * round(x / self.resolution)
        y = self.resolution * round(y / self.resolution)
        if [x, y] in self.polyline:
            if len(self.polyline) >= 3:
                self._submit_polygon_obstacle(self.polyline)
                self._clear_current_polyline()
            return
        self.polyline.append([x, y])
        self.gui_current_polygon_vertices.append(self.gui.add_disc(
            POINT_RADIUS, x, y, fill_color=QtCore.Qt.red))
        if len(self.polyline) > 1:
            self.gui_current_polygon_edges.append(
                self.gui.add_segment(*self.polyline[-2], *self.polyline[-1], line_color=QtCore.Qt.red))

    def _redraw_grid(self, size):
        for segment in self.grid:
            self.gui.scene.removeItem(segment.line)
        self.grid.clear()
        color = QtCore.Qt.lightGray
        length = size * self.resolution
        for i in range(-size, size):
            if i == 0:
                continue
            i = i * self.resolution
            self.grid.append(self.gui.add_segment(-length, i,
                                                  length, i, line_color=color))
            self.grid.append(self.gui.add_segment(
                i, -length, i, length, line_color=color))
        color = QtCore.Qt.black
        self.grid.append(self.gui.add_segment(-length, 0,
                         length, 0, line_color=color))
        self.grid.append(self.gui.add_segment(
            0, -length, 0, length, line_color=color))
        for rline in self.grid:
            rline.line.setZValue(-1)

    def _save_scene_button(self):
        self._save_scene(self.gui.get_field('scene_save'))

    def _save_scene(self, filename):
        if len(self.polygon_obstacles) != 1:
            self._display_print("invalid scene")
            return

        self.gui.set_field('scene_save', filename)
        # TODO support holes
        d = {'scene_boundary': self.polygon_obstacles[0]}
        try:
            with open(filename, 'w') as f:
                f.write(json.dumps(d, indent=4, sort_keys=True))
                self._display_print("Scene saved to:", filename)
        except Exception as e:
            self._display_print("Failed to write to file", filename, ':', e)

    def _load_scene_button(self):
        self._load_scene(self.gui.get_field('scene_load'))

    def _load_scene(self, filename, *args):
        self.gui.set_field('scene_load', filename)
        try:
            with open(filename, 'r') as f:
                d = json.load(f)
                self._clear()

                if 'obstacles' in d:
                    if len(d['obstacles']) > 1:
                        raise ValueError(
                            "only scene with one obstacle are supported")
                    for polygon in d['obstacles']:
                        if not self._submit_polygon_obstacle(polygon):
                            raise ValueError("polygon is not valid")
            self._display_print("Scene loaded from:", filename)
        except Exception as e:
            self._display_print("Failed to load file", filename, ': ', e)

    def _clear(self):
        for gui_polygon in self.gui_polygon_obstacles:
            self.gui.scene.removeItem(gui_polygon.polygon)
        self.gui_polygon_obstacles.clear()
        self.polygon_obstacles.clear()

    def _set_resolution(self):
        try:
            self.resolution = float(self.gui.get_field('resolution'))
            self._display_print("Resolution set to:", self.resolution)
        except Exception as e:
            self._display_print('Failed to set resolution:', e)
        self._redraw_grid(GRID_SIZE)

    def _undo(self):
        if len(self.polyline) == 0:
            return
        self.polyline.pop()
        self.gui.scene.removeItem(
            self.gui_current_polygon_vertices.pop().disc)
        if self.gui_current_polygon_edges:
            self.gui.scene.removeItem(
                self.gui_current_polygon_edges.pop().line)

    def _open_scene_load_dialog(self):
        initpath = os.path.dirname(self.gui.get_field('scene_load'))
        initpath = initpath if os.path.isdir(initpath) else ""
        file_path = open_file_dialog(initpath)
        if file_path:
            self._load_scene(file_path)

    def _open_scene_save_dialog(self):
        initpath = os.path.dirname(self.gui.get_field('scene_save'))
        if not os.path.isdir(initpath):
            initpath = os.path.dirname(self.gui.get_field('scene_load'))
        initpath = initpath if os.path.isdir(initpath) else ""
        file_path = open_file_dialog(initpath)
        if file_path:
            self._save_scene(file_path)

    def run(self):
        self.gui.mainWindow.show()
        return self._app.exec_()


if __name__ == '__main__':
    if not (sys.platform == "linux" or sys.platform == "linux2"):
        fdml_gui_appid = u'fdml.fdml_gui.scene_designer'
        ctypes.windll.shell32.SetCurrentProcessExplicitAppUserModelID(
            fdml_gui_appid)

    gui = SceneDesignerGUI(sys.argv)
    ret = gui.run()
    sys.exit(ret)
