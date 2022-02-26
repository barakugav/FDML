#!/usr/bin/env python3

import os
import sys
import json
from PyQt5 import QtCore, QtWidgets, QtGui
from PyQt5.QtWidgets import QFileDialog
import localizator
import numpy as np

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "gui/"))
from Worker import Worker
from logger import Logger, Writer
from gui import GUI


class LocalizatorGUI(GUI):
    def __init__(self):
        super().__init__()
        self.redraw()

    def setupUi(self):
        MainWindow = self.mainWindow
        MainWindow.setObjectName("MainWindow")
        MainWindow.resize(self.width, self.height)
        MainWindow.setStyleSheet("QMainWindow { background-color : rgb(54, 57, 63); color : rgb(220, 221, 222); }\n"
                                 "#centralwidget { background-color : rgb(54, 57, 63); color : rgb(220, 221, 222); }\n"
                                 "QLabel { background-color : rgb(54, 57, 63); color : rgb(220, 221, 222); }")
        self.centralwidget = QtWidgets.QWidget(MainWindow)
        self.centralwidget.setObjectName("centralwidget")
        self.gridLayout = QtWidgets.QGridLayout(self.centralwidget)
        self.gridLayout.setObjectName("gridLayout")
        self.graphicsView = QtWidgets.QGraphicsView(self.centralwidget)
        self.graphicsView.setEnabled(True)
        sizePolicy = QtWidgets.QSizePolicy(
            QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Expanding)
        sizePolicy.setHorizontalStretch(1)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(
            self.graphicsView.sizePolicy().hasHeightForWidth())
        self.graphicsView.setSizePolicy(sizePolicy)
        self.graphicsView.setObjectName("graphicsView")
        self.gridLayout.addWidget(self.graphicsView, 3, 2, 1, 1)
        self.gridLayout_0 = QtWidgets.QGridLayout()
        self.gridLayout_0.setObjectName("gridLayout_0")
        self.pushButton_compute = QtWidgets.QPushButton(self.centralwidget)
        font = QtGui.QFont()
        font.setPointSize(12)
        self.pushButton_compute.setFont(font)
        self.pushButton_compute.setObjectName("pushButton_compute")
        self.gridLayout_0.addWidget(self.pushButton_compute, 15, 0, 1, 1)
        self.lineEdit_scene = QtWidgets.QLineEdit(self.centralwidget)
        font = QtGui.QFont()
        font.setPointSize(12)
        self.lineEdit_scene.setFont(font)
        self.lineEdit_scene.setObjectName("lineEdit_scene")
        self.gridLayout_0.addWidget(self.lineEdit_scene, 4, 0, 1, 1)
        self.toolButton_searchScene = QtWidgets.QToolButton(self.centralwidget)
        font = QtGui.QFont()
        font.setPointSize(12)
        self.toolButton_searchScene.setFont(font)
        self.toolButton_searchScene.setObjectName("toolButton_searchScene")
        self.gridLayout_0.addWidget(self.toolButton_searchScene, 4, 1, 1, 1)
        self.label_scene = QtWidgets.QLabel(self.centralwidget)
        font = QtGui.QFont()
        font.setPointSize(12)
        self.label_scene.setFont(font)
        self.label_scene.setObjectName("label_scene")
        self.gridLayout_0.addWidget(self.label_scene, 3, 0, 1, 1)
        self.pushButton_scene = QtWidgets.QPushButton(self.centralwidget)
        font = QtGui.QFont()
        font.setPointSize(12)
        self.pushButton_scene.setFont(font)
        self.pushButton_scene.setObjectName("pushButton_scene")
        self.gridLayout_0.addWidget(self.pushButton_scene, 5, 0, 1, 1)
        spacerItem = QtWidgets.QSpacerItem(300, 0, QtWidgets.QSizePolicy.MinimumExpanding,
                                           QtWidgets.QSizePolicy.Minimum)
        self.gridLayout_0.addItem(spacerItem, 22, 0, 3, 1)
        self.label_d = QtWidgets.QLabel(self.centralwidget)
        font = QtGui.QFont()
        font.setPointSize(12)
        self.label_d.setFont(font)
        self.label_d.setObjectName("label_d")
        self.gridLayout_0.addWidget(self.label_d, 8, 0, 1, 1)
        self.lineEdit_d = QtWidgets.QLineEdit(self.centralwidget)
        font = QtGui.QFont()
        font.setPointSize(12)
        self.lineEdit_d.setFont(font)
        self.lineEdit_d.setObjectName("lineEdit_d")
        self.gridLayout_0.addWidget(self.lineEdit_d, 10, 0, 1, 1)
        self.gridLayout.addLayout(self.gridLayout_0, 3, 1, 1, 1)
        self.textEdit = Logger(self.centralwidget)
        self.textEdit.setObjectName("textEdit")
        self.gridLayout.addWidget(self.textEdit, 3, 0, 1, 1)
        MainWindow.setCentralWidget(self.centralwidget)
        self.statusbar = QtWidgets.QStatusBar(MainWindow)
        self.statusbar.setObjectName("statusbar")
        MainWindow.setStatusBar(self.statusbar)

        QtCore.QMetaObject.connectSlotsByName(MainWindow)

        self.lineEdits['scene'] = self.lineEdit_scene
        self.lineEdits['d'] = self.lineEdit_d
        self.pushButtons['scene'] = self.pushButton_scene
        self.pushButtons['compute'] = self.pushButton_compute
        self.pushButtons['searchScene'] = self.toolButton_searchScene
        self.labels['scene'] = self.label_scene
        self.labels['d'] = self.label_d


class Polygons_scene():
    def __init__(self, gui, writer):
        self.writer = writer
        self.gui = gui
        self.obstacles = []
        self.gui_obstacles = []
        self.path = []

    def draw_scene(self):
        self.gui.clear_scene()
        for obstacle in self.obstacles:
            self.gui_obstacles = []
            self.gui_obstacles.append(
                self.gui.add_polygon(obstacle, QtCore.Qt.darkGray))

    @staticmethod
    def read_scene(filename):
        obstacles = []
        with open(filename, "r") as f:
            d = json.load(f)
            if 'obstacles' in d:
                obstacles = d['obstacles']
        return obstacles

    def load_scene(self, filename):
        self.obstacles = []
        self.path = []
        try:
            self.obstacles = Polygons_scene.read_scene(filename)
            success = True
        except Exception as e:
            print('load_scene:', e, file=self.writer)
            success = False
        self.gui.empty_queue()
        self.draw_scene()
        print("Loaded scene from", filename, file=self.writer)
        return success


writer = None
loaded_scene = None
localizator = localizator.Localizator(os.path.join(os.getcwd(), ".localizator"))
res_polygons_gui = []


def clear_res_polygons_gui():
    for gui_polygon in res_polygons_gui:
        gui.scene.removeItem(gui_polygon.polygon)
    res_polygons_gui.clear()


def set_up_scene():
    localizator.stop()
    clear_res_polygons_gui()
    gui.clear_scene()
    scene_file = gui.get_field('scene')
    success = ps.load_scene(scene_file)
    loaded_scene = scene_file if success else None
    localizator.run(loaded_scene)


def compute():
    clear_res_polygons_gui()

    d = gui.get_field('d')
    try:
        d = float(d)
    except:
        print("invalid d value")
        return
    print("compute", d)

    res = localizator.query1(float(d))
    for polygon in res:
        fill_color = QtGui.QColor(0, 0, 255, 100)
        line_color = QtCore.Qt.transparent
        gui_polygon = gui.add_polygon(polygon, fill_color, line_color)
        res_polygons_gui.append(gui_polygon)


def enable():
    gui.set_label('scene', "Scene File (.json):")
    gui.set_logic('scene', set_up_scene)
    gui.set_button_text('scene', "Load Scene")
    gui.set_button_text('searchScene', "..")
    gui.set_logic('searchScene', set_input_file)
    gui.set_button_text('compute', "Compute")
    gui.set_logic('compute', compute)
    gui.set_label('d', "measurement size (d)")


def get_file():
    dlg = QFileDialog()
    dlg.setFileMode(QFileDialog.AnyFile)
    dlg.setDirectory('')
    if dlg.exec_():
        filenames = dlg.selectedFiles()
        return filenames[0]


def set_input_file():
    file_path = get_file()
    if file_path:
        gui.set_field('scene', file_path)


if __name__ == "__main__":
    app = QtWidgets.QApplication(sys.argv)
    gui = LocalizatorGUI()
    gui.set_program_name("Robot Localization")
    writer = Writer(gui.textEdit)

    enable()
    gui.mainWindow.signal_drop.connect(
        lambda path: gui.set_field('scene', path))
    threadpool = QtCore.QThreadPool()
    gui.set_animation_finished_action(enable)
    ps = Polygons_scene(gui, writer)
    gui.mainWindow.show()
    sys.exit(app.exec_())
