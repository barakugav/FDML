#!/usr/bin/env python3

import os
import sys
import json
import threading
from PyQt5 import QtCore, QtWidgets, QtGui
from PyQt5.QtWidgets import QFileDialog
import locator
import ctypes
from geometrygui.gui import GUI
from geometrygui.logger import Logger, Writer
from geometrygui.Worker import Worker


IMG_DIR = os.path.abspath(os.path.join(
    os.path.dirname(os.path.realpath(__file__)), "../img"))


class PolygonsScene():
    def __init__(self, gui, writer):
        self._writer = writer
        self._gui = gui
        self._obstacles = []
        self._gui_obstacles = []

    def draw_scene(self):
        self._gui.clear_scene()
        for obstacle in self._obstacles:
            self._gui_obstacles = []
            self._gui_obstacles.append(
                self._gui.add_polygon(obstacle, QtCore.Qt.darkGray))

    @staticmethod
    def read_scene(filename):
        scene_boundary = []
        with open(filename, "r") as f:
            d = json.load(f)
            if 'scene_boundary' in d:
                scene_boundary = [d['scene_boundary']]
        print("scene_boundary", scene_boundary)
        return scene_boundary

    def load_scene(self, filename):
        self._obstacles = []
        try:
            self._obstacles = PolygonsScene.read_scene(filename)
            success = True
        except Exception as e:
            print('load_scene:', e, file=self._writer)
            success = False
        self._gui.empty_queue()
        self.draw_scene()
        print("Loaded scene from", filename, file=self._writer)
        return success


class LocatorGUIComponent(GUI):
    def __init__(self):
        super().__init__()
        self.set_program_name("Robot Localization")
        self.mainWindow.setWindowIcon(
            QtGui.QIcon(os.path.join(IMG_DIR, "icon.png")))
        self.redraw()

    def setup_ui(self):
        main_window = self.mainWindow
        main_window.setObjectName("main_window")
        main_window.resize(self.width, self.height)
        main_window.setStyleSheet("QMainWindow { background-color : rgb(54, 57, 63); color : rgb(220, 221, 222); }\n"
                                  "#central_widget { background-color : rgb(54, 57, 63); color : rgb(220, 221, 222); }\n"
                                  "QLabel { background-color : rgb(54, 57, 63); color : rgb(220, 221, 222); }")
        self.central_widget = QtWidgets.QWidget(main_window)
        self.central_widget.setObjectName("central_widget")
        self.central_layout = QtWidgets.QGridLayout(self.central_widget)
        self.central_layout.setObjectName("central_layout")
        self.graphics_view = QtWidgets.QGraphicsView(self.central_widget)
        self.graphics_view.setEnabled(True)
        size_policy = QtWidgets.QSizePolicy(
            QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Expanding)
        size_policy.setHorizontalStretch(1)
        size_policy.setVerticalStretch(0)
        size_policy.setHeightForWidth(
            self.graphics_view.sizePolicy().hasHeightForWidth())
        self.graphics_view.setSizePolicy(size_policy)
        self.graphics_view.setObjectName("graphics_view")
        self.central_layout.addWidget(self.graphics_view, 3, 2, 1, 1)

        # actions panel
        self._setup_ui_actions_panel()

        # logger
        self.logger = Logger(self.central_widget)
        self.logger.setObjectName("logger")
        self.central_layout.addWidget(self.logger, 3, 0, 1, 1)

        main_window.setCentralWidget(self.central_widget)

        # TODO maybe remove
        self.statusbar = QtWidgets.QStatusBar(main_window)
        self.statusbar.setObjectName("statusbar")
        main_window.setStatusBar(self.statusbar)

        QtCore.QMetaObject.connectSlotsByName(main_window)

    def _setup_ui_actions_panel(self):
        self.actions_panel_layout = QtWidgets.QGridLayout()
        self.actions_panel_layout.setObjectName("actions_panel_layout")
        font = QtGui.QFont()
        font.setPointSize(12)

        # == Scene loading ==
        # scene label
        self.scene_label = QtWidgets.QLabel(self.central_widget)
        self.scene_label.setFont(font)
        self.scene_label.setObjectName("scene_label")
        self.actions_panel_layout.addWidget(self.scene_label, 1, 0, 1, 1)
        # scene input text
        self.scene_intxt = QtWidgets.QLineEdit(self.central_widget)
        self.scene_intxt.setFont(font)
        self.scene_intxt.setObjectName("scene_intxt")
        self.actions_panel_layout.addWidget(self.scene_intxt, 2, 0, 1, 1)
        # scene dialog button
        self.scene_dialog_button = QtWidgets.QToolButton(self.central_widget)
        self.scene_dialog_button.setFont(font)
        self.scene_dialog_button.setObjectName("scene_dialog_button")
        self.actions_panel_layout.addWidget(
            self.scene_dialog_button, 2, 1, 1, 1)
        # scene load button
        self.scene_load_button = QtWidgets.QPushButton(self.central_widget)
        self.scene_load_button.setFont(font)
        self.scene_load_button.setObjectName("scene_load_button")
        self.actions_panel_layout.addWidget(self.scene_load_button, 3, 0, 1, 1)

        spacerItem = QtWidgets.QSpacerItem(200, 0, QtWidgets.QSizePolicy.MinimumExpanding,
                                           QtWidgets.QSizePolicy.Minimum)
        self.actions_panel_layout.addItem(spacerItem, 4, 0, 1, 1)

        # == Single measurement query ==
        # label
        self.m1_label = QtWidgets.QLabel(self.central_widget)
        self.m1_label.setFont(font)
        self.m1_label.setObjectName("m1_label")
        self.actions_panel_layout.addWidget(self.m1_label, 5, 0, 1, 1)
        # d input text
        self.m1_d_intxt = QtWidgets.QLineEdit(self.central_widget)
        self.m1_d_intxt.setFont(font)
        self.m1_d_intxt.setObjectName("m1_d_intxt")
        self.actions_panel_layout.addWidget(self.m1_d_intxt, 6, 0, 1, 1)
        # compute button
        self.m1_compute_button = QtWidgets.QPushButton(self.central_widget)
        self.m1_compute_button.setFont(font)
        self.m1_compute_button.setObjectName("m1_compute_button")
        self.actions_panel_layout.addWidget(self.m1_compute_button, 7, 0, 1, 1)

        spacerItem = QtWidgets.QSpacerItem(200, 0, QtWidgets.QSizePolicy.MinimumExpanding,
                                           QtWidgets.QSizePolicy.Minimum)
        self.actions_panel_layout.addItem(spacerItem, 8, 0, 1, 1)

        # == Two measurement query ==
        # label
        self.m2_label = QtWidgets.QLabel(self.central_widget)
        self.m2_label.setFont(font)
        self.m2_label.setObjectName("m2_label")
        self.actions_panel_layout.addWidget(self.m2_label, 9, 0, 1, 1)
        # d1 input test
        self.m2_d1_intxt = QtWidgets.QLineEdit(self.central_widget)
        self.m2_d1_intxt.setFont(font)
        self.m2_d1_intxt.setObjectName("m2_d1_intxt")
        self.actions_panel_layout.addWidget(self.m2_d1_intxt, 10, 0, 1, 1)
        # d2 input test
        self.m2_d2_intxt = QtWidgets.QLineEdit(self.central_widget)
        self.m2_d2_intxt.setFont(font)
        self.m2_d2_intxt.setObjectName("m2_d2_intxt")
        self.actions_panel_layout.addWidget(self.m2_d2_intxt, 11, 0, 1, 1)
        # compute button
        self.m2_compute_button = QtWidgets.QPushButton(self.central_widget)
        self.m2_compute_button.setFont(font)
        self.m2_compute_button.setObjectName("m2_compute_button")
        self.actions_panel_layout.addWidget(
            self.m2_compute_button, 12, 0, 1, 1)

        spacerItem = QtWidgets.QSpacerItem(200, 0, QtWidgets.QSizePolicy.MinimumExpanding,
                                           QtWidgets.QSizePolicy.Minimum)
        self.actions_panel_layout.addItem(spacerItem, 13, 0, 1, 1)

        # == misc
        # Clear label
        self.clear_label = QtWidgets.QLabel(self.central_widget)
        self.clear_label.setFont(font)
        self.clear_label.setObjectName("clear_label")
        self.actions_panel_layout.addWidget(self.clear_label, 14, 0, 1, 1)
        # Clear button
        self.clear_button = QtWidgets.QPushButton(self.central_widget)
        self.clear_button.setFont(font)
        self.clear_button.setObjectName("clear_button")
        self.actions_panel_layout.addWidget(self.clear_button, 15, 0, 1, 1)

        spacerItem = QtWidgets.QSpacerItem(200, 0, QtWidgets.QSizePolicy.MinimumExpanding,
                                           QtWidgets.QSizePolicy.Minimum)
        self.actions_panel_layout.addItem(spacerItem, 16, 0, 2, 1)

        self.central_layout.addLayout(self.actions_panel_layout, 3, 1, 1, 1)

        self.labels['scene'] = self.scene_label
        self.lineEdits['scene'] = self.scene_intxt
        self.pushButtons['scene_open_dialog'] = self.scene_dialog_button
        self.pushButtons['scene_load'] = self.scene_load_button
        self.set_label('scene', "Scene File (.json):")
        self.set_button_text('scene_open_dialog', "..")
        self.set_button_text('scene_load', "Load Scene")

        self.labels['m1_label'] = self.m1_label
        self.lineEdits['m1_d'] = self.m1_d_intxt
        self.pushButtons['m1_compute'] = self.m1_compute_button
        self.set_label('m1_label', "Single measurement value (d)")
        self.set_button_text('m1_compute', "Compute Single measurement")

        self.labels['m2_label'] = self.m2_label
        self.lineEdits['m2_d1'] = self.m2_d1_intxt
        self.lineEdits['m2_d2'] = self.m2_d2_intxt
        self.pushButtons['m2_compute'] = self.m2_compute_button
        self.set_label('m2_label', "Double measurement values (d1, d2)")
        self.set_button_text('m2_compute', "Compute Double measurement")

        self.labels['clear_label'] = self.clear_label
        self.pushButtons['clear_button'] = self.clear_button
        self.set_button_text('clear_button', "Clear")


class LocatorGUI:
    def __init__(self, args=[]):
        self._app = QtWidgets.QApplication(args)
        self._locator = locator.Locator()
        self._locator_worker = None
        self._locator_worker_lock = threading.Lock()
        self._locator_query1_res = None
        self._locator_query2_res = None
        self._gui_comp = LocatorGUIComponent()
        self._writer = Writer(self._gui_comp.logger)
        self._polygon_scene = PolygonsScene(self._gui_comp, self._writer)
        self._displayed_polygons = []
        self._displayed_segments = []
        self._threadpool = QtCore.QThreadPool()

        self._setup_gui_logic()
        self._gui_comp.mainWindow.close_handlers.append(
            self._finalize_locator)
        self._gui_comp.mainWindow.signal_drop.connect(
            lambda path: self._gui_comp.set_field('scene', path))

    def _print(self, *args):
        if self._writer:
            print(*args, file=self._writer)
        else:
            print(*args)

    def _load_scene(self):
        with self._locator_worker_lock:
            if self._locator_worker is not None:
                self._print("last command is still in proccessing...")
                return
            self._locator_worker = {}  # dummy place holder

        self._locator.stop()
        self._clear_displayed_result()
        self._gui_comp.clear_scene()
        scene_file = self._gui_comp.get_field('scene')
        success = self._polygon_scene.load_scene(scene_file)
        if not success:
            with self._locator_worker_lock:
                self._locator_worker = None
            return

        with self._locator_worker_lock:
            self._locator_worker = Worker(
                self._locator_init, scene_file)
            self._locator_worker.signals.finished.connect(
                self._locator_init_done)
        self._threadpool.start(self._locator_worker)

    def _locator_init(self, scene_filename, is_running):
        self._print("Locator init...")
        self._locator.run(scene_filename)

    def _locator_init_done(self, is_running):
        self._print("Locator init is done")
        with self._locator_worker_lock:
            self._locator_worker = None

    def _query1(self):
        self._clear_displayed_result()

        d = self._gui_comp.get_field('m1_d')
        try:
            d = float(d)
        except:
            d = -1
        if d <= 0:
            self._print("invalid d value")
            return

        with self._locator_worker_lock:
            if self._locator_worker is not None:
                self._print("last command is still in proccessing...")
                return
            self._locator_worker = Worker(self._locator_query1, d)
            self._locator_worker.signals.finished.connect(
                self._locator_query1_done)
        self._threadpool.start(self._locator_worker)

    def _locator_query1(self, d, is_running):
        self._print("Locator query with d = ", d)
        if not self._locator.is_running():
            self._print("Locator was not initialized")
            return
        self._locator_query1_res = self._locator.query1(d)

    def _locator_query1_done(self, is_running):
        if self._locator_query1_res is not None:
            self._print("Locator query is complete")
            for polygon in self._locator_query1_res:
                fill_color = QtGui.QColor(0, 0, 255, 100)
                line_color = QtCore.Qt.transparent
                gui_polygon = self._gui_comp.add_polygon(
                    polygon, fill_color, line_color)
                self._displayed_polygons.append(gui_polygon)
            self._locator_query1_res = None
        with self._locator_worker_lock:
            self._locator_worker = None

    def _query2(self):
        self._clear_displayed_result()

        d1 = self._gui_comp.get_field('m2_d1')
        d2 = self._gui_comp.get_field('m2_d2')
        try:
            d1, d2 = float(d1), float(d2)
        except:
            d1, d2 = -1, -1
        if d1 <= 0 or d2 <= 0:
            self._print("invalid d1 d2 values")
            return

        with self._locator_worker_lock:
            if self._locator_worker is not None:
                self._print("last command is still in proccessing...")
                return
            self._locator_worker = Worker(self._locator_query2, d1, d2)
            self._locator_worker.signals.finished.connect(
                self._locator_query2_done)
        self._threadpool.start(self._locator_worker)

    def _locator_query2(self, d1, d2, is_running):
        self._print("Locator query with d1 =", d1, "d2 =", d2)
        if not self._locator.is_running():
            self._print("Locator was not initialized")
            return
        self._locator_query2_res = self._locator.query2(d1, d2)

    def _locator_query2_done(self, is_running):
        if self._locator_query2_res is not None:
            self._print("Locator query is complete")
            for segment in self._locator_query2_res:
                line_color = QtGui.QColor(0, 0, 255, 100)
                gui_segment = self._gui_comp.add_segment(
                    segment[0][0], segment[0][1], segment[1][0], segment[1][1], line_color)
                self._displayed_segments.append(gui_segment)
            self._locator_query2_res = []
        with self._locator_worker_lock:
            self._locator_worker = None

    def _clear_displayed_result(self):
        for gui_polygon in self._displayed_polygons:
            self._gui_comp.scene.removeItem(gui_polygon.polygon)
        self._displayed_polygons.clear()
        for gui_segment in self._displayed_segments:
            self._gui_comp.scene.removeItem(gui_segment.line)
        self._displayed_segments.clear()

    def _open_scene_dialog(self):
        dlg = QFileDialog()
        dlg.setFileMode(QFileDialog.AnyFile)
        dlg.setDirectory('')
        if dlg.exec_():
            file_path = dlg.selectedFiles()[0]
            self._gui_comp.set_field('scene', file_path)

    def _finalize_locator(self):
        self._locator.stop()

    def _setup_gui_logic(self):
        self._gui_comp.set_logic('scene_open_dialog', self._open_scene_dialog)
        self._gui_comp.set_logic('scene_load', self._load_scene)
        self._gui_comp.set_logic('m1_compute', self._query1)
        self._gui_comp.set_logic('m2_compute', self._query2)
        self._gui_comp.set_logic('clear_button', self._clear_displayed_result)

    def run(self):
        self._gui_comp.mainWindow.show()
        return self._app.exec_()


def main():
    if not (sys.platform == "linux" or sys.platform == "linux2"):
        fdml_gui_appid = u'fdml.fdml_gui.locator'
        ctypes.windll.shell32.SetCurrentProcessExplicitAppUserModelID(
            fdml_gui_appid)

    gui = LocatorGUI(sys.argv)
    ret = gui.run()
    sys.exit(ret)

if __name__ == "__main__":
    main()
