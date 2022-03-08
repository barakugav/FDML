#!/usr/bin/env python3

import sys
import os
import json
from PyQt5 import QtCore, QtGui, QtWidgets
from PyQt5.QtCore import pyqtSignal
from PyQt5.QtWidgets import QFileDialog

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "gui/"))
from gui import GUI, MainWindowPlus, RPolygon, RSegment, RDisc


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


class GUI_scene_designer(GUI):
    textEdits = []

    def __init__(self):
        super().__init__()
        self.zoom = 50.0
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
        self.pushButton_resolution = QtWidgets.QPushButton(self.centralwidget)
        font = QtGui.QFont()
        font.setPointSize(12)
        self.pushButton_resolution.setFont(font)
        self.pushButton_resolution.setObjectName("pushButton_resolution")
        spacerItem = QtWidgets.QSpacerItem(300, 20, QtWidgets.QSizePolicy.MinimumExpanding,
                                           QtWidgets.QSizePolicy.Minimum)
        self.gridLayout_0.addItem(spacerItem, 33, 0, 1, 1)
        self.label_saveLocation = QtWidgets.QLabel(self.centralwidget)
        font = QtGui.QFont()
        font.setPointSize(12)
        self.label_saveLocation.setFont(font)
        self.label_saveLocation.setObjectName("label_saveLocation")
        self.gridLayout_0.addWidget(self.label_saveLocation, 4, 0, 1, 1)
        self.lineEdit_scene = QtWidgets.QLineEdit(self.centralwidget)
        font = QtGui.QFont()
        font.setPointSize(12)
        self.lineEdit_scene.setFont(font)
        self.lineEdit_scene.setObjectName("lineEdit_scene")
        self.gridLayout_0.addWidget(self.lineEdit_scene, 2, 0, 1, 1)
        self.lineEdit_location = QtWidgets.QLineEdit(self.centralwidget)
        font = QtGui.QFont()
        font.setPointSize(12)
        self.lineEdit_location.setFont(font)
        self.lineEdit_location.setObjectName("lineEdit_location")
        self.gridLayout_0.addWidget(self.lineEdit_location, 5, 0, 1, 1)
        self.label_scene = QtWidgets.QLabel(self.centralwidget)
        font = QtGui.QFont()
        font.setPointSize(12)
        self.label_scene.setFont(font)
        self.label_scene.setObjectName("label_scene")
        self.gridLayout_0.addWidget(self.label_scene, 1, 0, 1, 1)
        self.textEdit = QtWidgets.QTextEdit(self.centralwidget)
        self.textEdit.setObjectName("textEdit")
        self.gridLayout_0.addWidget(self.textEdit, 9, 0, 1, 1)
        self.pushButton_save = QtWidgets.QPushButton(self.centralwidget)
        font = QtGui.QFont()
        font.setPointSize(12)
        self.pushButton_save.setFont(font)
        self.pushButton_save.setObjectName("pushButton_save")
        self.gridLayout_0.addWidget(self.pushButton_save, 8, 0, 1, 1)
        self.label_resolution = QtWidgets.QLabel(self.centralwidget)
        font = QtGui.QFont()
        font.setPointSize(12)
        self.label_resolution.setFont(font)
        self.label_resolution.setObjectName("label_resolution")
        self.gridLayout_0.addWidget(self.label_resolution, 13, 0, 1, 1)
        self.lineEdit_resolution = QtWidgets.QLineEdit(self.centralwidget)
        font = QtGui.QFont()
        font.setPointSize(12)
        self.lineEdit_resolution.setFont(font)
        self.lineEdit_resolution.setObjectName("lineEdit_resolution")
        self.gridLayout_0.addWidget(self.lineEdit_resolution, 14, 0, 1, 1)
        self.toolButton_searchScene = QtWidgets.QToolButton(self.centralwidget)
        font = QtGui.QFont()
        font.setPointSize(12)
        self.toolButton_searchScene.setFont(font)
        self.toolButton_searchScene.setObjectName("toolButton_searchScene")
        self.gridLayout_0.addWidget(self.toolButton_searchScene, 2, 1, 1, 1)
        self.pushButton_clear = QtWidgets.QPushButton(self.centralwidget)
        font = QtGui.QFont()
        font.setPointSize(12)
        self.pushButton_clear.setFont(font)
        self.pushButton_clear.setObjectName("pushButton_clear")
        self.gridLayout_0.addWidget(self.pushButton_clear, 29, 0, 1, 1)
        spacerItem1 = QtWidgets.QSpacerItem(
            20, 40, QtWidgets.QSizePolicy.Minimum, QtWidgets.QSizePolicy.Expanding)
        self.gridLayout_0.addItem(spacerItem1, 30, 0, 1, 1)
        self.pushButton_load = QtWidgets.QPushButton(self.centralwidget)
        font = QtGui.QFont()
        font.setPointSize(12)
        self.pushButton_load.setFont(font)
        self.pushButton_load.setObjectName("pushButton_load")
        self.gridLayout_0.addWidget(self.pushButton_load, 3, 0, 1, 1)
        self.gridLayout.addLayout(self.gridLayout_0, 3, 0, 1, 1)
        main_window.setCentralWidget(self.centralwidget)
        self.statusbar = QtWidgets.QStatusBar(main_window)
        self.statusbar.setObjectName("statusbar")
        main_window.setStatusBar(self.statusbar)

        QtCore.QMetaObject.connectSlotsByName(main_window)

        self.lineEdits['scene'] = self.lineEdit_scene
        self.lineEdits['saveLocation'] = self.lineEdit_location
        self.lineEdits['resolution'] = self.lineEdit_resolution
        self.pushButtons['load'] = self.pushButton_load
        self.pushButtons['save'] = self.pushButton_save
        self.pushButtons['resolution'] = self.pushButton_resolution
        self.pushButtons['clear'] = self.pushButton_clear
        self.pushButtons['searchScene'] = self.toolButton_searchScene
        self.labels['scene'] = self.label_scene
        self.labels['saveLocation'] = self.label_saveLocation
        self.labels['resolution'] = self.label_resolution


colors = [QtCore.Qt.yellow, QtCore.Qt.green, QtCore.Qt.red, QtCore.Qt.magenta,
          QtCore.Qt.darkYellow, QtCore.Qt.darkGreen, QtCore.Qt.darkRed, QtCore.Qt.darkMagenta]
current_color_index = 0

POINT_RADIUS = 0.1
GRID_SIZE = 200
resolution = 1.0
polygon_obstacles = []
gui_polygon_obstacles = []
polyline = []
gui_current_polygon_vertices = []
gui_current_polygon_edges = []
grid = []


def clear_current_polyline():
    global polyline
    polyline = []
    vertex: RDisc
    for vertex in gui_current_polygon_vertices:
        gui.scene.removeItem(vertex.disc)
    gui_current_polygon_vertices.clear()
    edge: RSegment
    for edge in gui_current_polygon_edges:
        gui.scene.removeItem(edge.line)
    gui_current_polygon_edges.clear()


def submit_polygon_obstacle(polygon):
    if not is_simple_polygon(polygon):
        print("invalid polygon!")
        return False
    polygon_obstacles.append(polygon)
    gui_polygon_obstacles.append(gui.add_polygon(
        polygon, fill_color=QtCore.Qt.transparent, line_color=QtCore.Qt.blue))
    return True


def cancel_current_polygon():
    polyline.clear()
    for vertex in gui_current_polygon_vertices:
        gui.scene.removeItem(vertex.disc)
    gui_current_polygon_vertices.clear()
    edge: RSegment
    for edge in gui_current_polygon_edges:
        gui.scene.removeItem(edge.line)
    gui_current_polygon_edges.clear()


def pop_vertex():
    index = len(polyline) - 1
    if 0 <= index < len(polyline):
        polyline.pop()
        gui.scene.removeItem(gui_current_polygon_vertices.pop().disc)
        if gui_current_polygon_edges:
            gui.scene.removeItem(gui_current_polygon_edges.pop().line)


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


def right_click(x: float, y: float):
    if len(polygon_obstacles) > 0:
        clear()
    print('x', x)
    print('y', y)
    x = resolution * round(x / resolution)
    y = resolution * round(y / resolution)
    print(x, y)
    if [x, y] in polyline:
        if len(polyline) >= 3:
            submit_polygon_obstacle(polyline)
            clear_current_polyline()
        return
    polyline.append([x, y])
    gui_current_polygon_vertices.append(gui.add_disc(
        POINT_RADIUS, x, y, fill_color=QtCore.Qt.red))
    if len(polyline) > 1:
        gui_current_polygon_edges.append(
            gui.add_segment(*polyline[-2], *polyline[-1], line_color=QtCore.Qt.red))


def redraw_grid(size):
    for segment in grid:
        gui.scene.removeItem(segment.line)
    grid.clear()
    color = QtCore.Qt.lightGray
    # size = int(size/RESOLUTION)
    length = size * resolution
    for i in range(-size, size):
        if i == 0:
            continue
        i = i * resolution
        grid.append(gui.add_segment(-length, i, length, i, line_color=color))
        grid.append(gui.add_segment(i, -length, i, length, line_color=color))
    color = QtCore.Qt.black
    grid.append(gui.add_segment(-length, 0, length, 0, line_color=color))
    grid.append(gui.add_segment(0, -length, 0, length, line_color=color))
    for rline in grid:
        rline.line.setZValue(-1)


def set_up():
    redraw_grid(GRID_SIZE)
    gui.add_disc(POINT_RADIUS, 0, 0)


def export_scene():
    filename = gui.get_field('saveLocation')
    d = {'obstacles': polygon_obstacles, }
    try:
        with open(filename, 'w') as f:
            f.write(json.dumps(d, indent=4, sort_keys=True))
            gui.textEdit.setPlainText("Scene saved to: " + filename)
    except Exception as e:
        print('Failed to write to file', filename + ':', e)
        gui.textEdit.setPlainText(
            'Failed to write to file ' + filename + ':' + str(e))


def load_scene():
    global polyline
    global current_color_index
    filename = gui.get_field('scene')
    try:
        with open(filename, 'r') as f:
            d = json.load(f)
            clear()

            if 'obstacles' in d:
                if len(d['obstacles']) > 1:
                    raise ValueError(
                        "only scene with one obstacle are supported")
                for polygon in d['obstacles']:
                    if not submit_polygon_obstacle(polygon):
                        raise ValueError("polygon is not valid")
        gui.textEdit.setPlainText("Scene loaded from: " + filename)
    except Exception as e:
        print('Failed to load from file', filename + ':', e)
        gui.textEdit.setPlainText(
            'Failed to load file ' + filename + ': ' + str(e))


def clear():
    gui_polygon: RPolygon
    for gui_polygon in gui_polygon_obstacles:
        gui.scene.removeItem(gui_polygon.polygon)
    gui_polygon_obstacles.clear()
    polygon_obstacles.clear()


def set_resolution():
    global resolution
    try:
        resolution = float(gui.get_field('resolution'))
        print('Resolution set to:', resolution)
        gui.textEdit.setPlainText('Resolution set to: ' + str(resolution))
    except Exception as e:
        print('Failed to set resolution:', e)
    redraw_grid(GRID_SIZE)


def undo():
    global selected
    if polyline:
        pop_vertex()


def get_file():
    dlg = QFileDialog()
    dlg.setFileMode(QFileDialog.AnyFile)
    dlg.setDirectory('')
    if dlg.exec_():
        filenames = dlg.selectedFiles()
        return filenames[0]


def browse_input_file():
    file_path = get_file()
    if file_path:
        gui.set_field('scene', file_path)
        load_scene()


def drop_input_file(file_path):
    gui.set_field('scene', file_path)
    load_scene()


if __name__ == '__main__':
    if len(sys.argv) > 1:
        resolution = float(sys.argv[1])
    app = QtWidgets.QApplication(sys.argv)
    gui = GUI_scene_designer()
    gui.scene.right_click_signal.connect(right_click)
    gui.mainWindow.signal_ctrl_z.connect(undo)
    gui.mainWindow.signal_esc.connect(cancel_current_polygon)
    gui.mainWindow.signal_drop.connect(drop_input_file)
    gui.set_animation_finished_action(lambda: None)
    gui.set_label('scene', 'Input Path:')
    gui.set_logic('load', load_scene)
    gui.set_button_text('load', 'Load Scene')
    gui.set_label('saveLocation', 'Output Path:')
    gui.set_logic('save', export_scene)
    gui.set_button_text('save', 'Save Scene')
    gui.set_label('resolution', 'Resolution')
    gui.set_field('resolution', str(resolution))
    gui.set_logic('resolution', set_resolution)
    gui.set_button_text('resolution', 'Set Resolution')
    gui.set_button_text('searchScene', '..')
    gui.set_logic('searchScene', browse_input_file)
    gui.set_button_text('clear', 'Clear scene')
    gui.set_logic('clear', clear)
    set_up()
    gui.mainWindow.show()
    sys.exit(app.exec_())
