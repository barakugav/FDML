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
    # comboBoxes = {}

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
        # self.lineEdit_2 = QtWidgets.QLineEdit(self.centralwidget)
        # font = QtGui.QFont()
        # font.setPointSize(12)
        # self.lineEdit_2.setFont(font)
        # self.lineEdit_2.setObjectName("lineEdit_2")
        # self.gridLayout_0.addWidget(self.lineEdit_2, 14, 0, 1, 1)
        self.pushButton_2 = QtWidgets.QPushButton(self.centralwidget)
        font = QtGui.QFont()
        font.setPointSize(12)
        self.pushButton_2.setFont(font)
        self.pushButton_2.setObjectName("pushButton_2")
        self.gridLayout_0.addWidget(self.pushButton_2, 15, 0, 1, 1)
        # self.pushButton_1 = QtWidgets.QPushButton(self.centralwidget)
        # font = QtGui.QFont()
        # font.setPointSize(12)
        # self.pushButton_1.setFont(font)
        # self.pushButton_1.setObjectName("pushButton_1")
        # self.gridLayout_0.addWidget(self.pushButton_1, 11, 0, 1, 1)
        # self.label_1 = QtWidgets.QLabel(self.centralwidget)
        # font = QtGui.QFont()
        # font.setPointSize(12)
        # self.label_1.setFont(font)
        # self.label_1.setObjectName("label_1")
        # self.gridLayout_0.addWidget(self.label_1, 6, 0, 1, 1)
        self.lineEdit_0 = QtWidgets.QLineEdit(self.centralwidget)
        font = QtGui.QFont()
        font.setPointSize(12)
        self.lineEdit_0.setFont(font)
        self.lineEdit_0.setObjectName("lineEdit_0")
        self.gridLayout_0.addWidget(self.lineEdit_0, 4, 0, 1, 1)
        # self.comboBox = QtWidgets.QComboBox(self.centralwidget)
        # self.comboBox.setObjectName("comboBox")
        # self.gridLayout_0.addWidget(self.comboBox, 2, 0, 1, 1)
        # self.toolButton_1 = QtWidgets.QToolButton(self.centralwidget)
        # font = QtGui.QFont()
        # font.setPointSize(12)
        # self.toolButton_1.setFont(font)
        # self.toolButton_1.setObjectName("toolButton_1")
        # self.gridLayout_0.addWidget(self.toolButton_1, 7, 1, 1, 1)
        # self.toolButton_2 = QtWidgets.QToolButton(self.centralwidget)
        # font = QtGui.QFont()
        # font.setPointSize(12)
        # self.toolButton_2.setFont(font)
        # self.toolButton_2.setObjectName("toolButton_2")
        # self.gridLayout_0.addWidget(self.toolButton_2, 14, 1, 1, 1)
        # self.lineEdit_1 = QtWidgets.QLineEdit(self.centralwidget)
        # font = QtGui.QFont()
        # font.setPointSize(12)
        # self.lineEdit_1.setFont(font)
        # self.lineEdit_1.setObjectName("lineEdit_1")
        # self.gridLayout_0.addWidget(self.lineEdit_1, 7, 0, 1, 1)
        # self.pushButton_4 = QtWidgets.QPushButton(self.centralwidget)
        # font = QtGui.QFont()
        # font.setPointSize(12)
        # self.pushButton_4.setFont(font)
        # self.pushButton_4.setObjectName("pushButton_4")
        # self.gridLayout_0.addWidget(self.pushButton_4, 20, 0, 1, 1)
        self.toolButton_0 = QtWidgets.QToolButton(self.centralwidget)
        font = QtGui.QFont()
        font.setPointSize(12)
        self.toolButton_0.setFont(font)
        self.toolButton_0.setObjectName("toolButton_0")
        self.gridLayout_0.addWidget(self.toolButton_0, 4, 1, 1, 1)
        # self.pushButton_8 = QtWidgets.QPushButton(self.centralwidget)
        # font = QtGui.QFont()
        # font.setPointSize(12)
        # self.pushButton_8.setFont(font)
        # self.pushButton_8.setObjectName("pushButton_8")
        # self.gridLayout_0.addWidget(self.pushButton_8, 18, 0, 1, 1)
        # self.label_7 = QtWidgets.QLabel(self.centralwidget)
        # font = QtGui.QFont()
        # font.setPointSize(12)
        # self.label_7.setFont(font)
        # self.label_7.setObjectName("label_7")
        # self.gridLayout_0.addWidget(self.label_7, 1, 0, 1, 1)
        self.label_0 = QtWidgets.QLabel(self.centralwidget)
        font = QtGui.QFont()
        font.setPointSize(12)
        self.label_0.setFont(font)
        self.label_0.setObjectName("label_0")
        self.gridLayout_0.addWidget(self.label_0, 3, 0, 1, 1)
        # self.lineEdit_5 = QtWidgets.QLineEdit(self.centralwidget)
        # font = QtGui.QFont()
        # font.setPointSize(12)
        # self.lineEdit_5.setFont(font)
        # self.lineEdit_5.setObjectName("lineEdit_5")
        # self.gridLayout_0.addWidget(self.lineEdit_5, 17, 0, 1, 1)
        # self.pushButton_3 = QtWidgets.QPushButton(self.centralwidget)
        # font = QtGui.QFont()
        # font.setPointSize(12)
        # self.pushButton_3.setFont(font)
        # self.pushButton_3.setObjectName("pushButton_3")
        # self.gridLayout_0.addWidget(self.pushButton_3, 19, 0, 1, 1)
        self.pushButton_0 = QtWidgets.QPushButton(self.centralwidget)
        font = QtGui.QFont()
        font.setPointSize(12)
        self.pushButton_0.setFont(font)
        self.pushButton_0.setObjectName("pushButton_0")
        self.gridLayout_0.addWidget(self.pushButton_0, 5, 0, 1, 1)
        # self.label_5 = QtWidgets.QLabel(self.centralwidget)
        # font = QtGui.QFont()
        # font.setPointSize(12)
        # self.label_5.setFont(font)
        # self.label_5.setObjectName("label_5")
        # self.gridLayout_0.addWidget(self.label_5, 16, 0, 1, 1)
        spacerItem = QtWidgets.QSpacerItem(300, 0, QtWidgets.QSizePolicy.MinimumExpanding,
                                           QtWidgets.QSizePolicy.Minimum)
        self.gridLayout_0.addItem(spacerItem, 22, 0, 3, 1)
        # self.pushButton_7 = QtWidgets.QPushButton(self.centralwidget)
        # font = QtGui.QFont()
        # font.setPointSize(12)
        # self.pushButton_7.setFont(font)
        # self.pushButton_7.setObjectName("pushButton_7")
        # self.gridLayout_0.addWidget(self.pushButton_7, 0, 0, 1, 1)
        self.label_4 = QtWidgets.QLabel(self.centralwidget)
        font = QtGui.QFont()
        font.setPointSize(12)
        self.label_4.setFont(font)
        self.label_4.setObjectName("label_4")
        self.gridLayout_0.addWidget(self.label_4, 8, 0, 1, 1)
        self.lineEdit_4 = QtWidgets.QLineEdit(self.centralwidget)
        font = QtGui.QFont()
        font.setPointSize(12)
        self.lineEdit_4.setFont(font)
        self.lineEdit_4.setObjectName("lineEdit_4")
        self.gridLayout_0.addWidget(self.lineEdit_4, 10, 0, 1, 1)
        # self.label_2 = QtWidgets.QLabel(self.centralwidget)
        # font = QtGui.QFont()
        # font.setPointSize(12)
        # self.label_2.setFont(font)
        # self.label_2.setObjectName("label_2")
        # self.gridLayout_0.addWidget(self.label_2, 13, 0, 1, 1)
        self.gridLayout.addLayout(self.gridLayout_0, 3, 1, 1, 1)
        self.textEdit = Logger(self.centralwidget)
        self.textEdit.setObjectName("textEdit")
        self.gridLayout.addWidget(self.textEdit, 3, 0, 1, 1)
        MainWindow.setCentralWidget(self.centralwidget)
        self.statusbar = QtWidgets.QStatusBar(MainWindow)
        self.statusbar.setObjectName("statusbar")
        MainWindow.setStatusBar(self.statusbar)

        QtCore.QMetaObject.connectSlotsByName(MainWindow)

        # end of eq designer generated code

        self.lineEdits['scene'] = self.lineEdit_0
        # self.lineEdits['planner'] = self.lineEdit_1
        self.lineEdits['d'] = self.lineEdit_4
        # self.lineEdits['solution'] = self.lineEdit_2
        # self.lineEdits['export'] = self.lineEdit_5
        self.pushButtons['scene'] = self.pushButton_0
        # self.pushButtons['planner'] = self.pushButton_1
        self.pushButtons['compute'] = self.pushButton_2
        # self.pushButtons['animate'] = self.pushButton_3
        # self.pushButtons['verify'] = self.pushButton_4
        # self.pushButtons['compute'] = self.pushButton_7
        # self.pushButtons['export'] = self.pushButton_8
        self.pushButtons['searchScene'] = self.toolButton_0
        # self.pushButtons['searchPlanner'] = self.toolButton_1
        # self.pushButtons['searchSolution'] = self.toolButton_2
        self.labels['scene'] = self.label_0
        # self.labels['planner'] = self.label_1
        # self.labels['solution'] = self.label_2
        self.labels['d'] = self.label_4
        # self.labels['export'] = self.label_5
        # self.labels['mode'] = self.label_7

        # self.comboBoxes['mode'] = self.comboBox


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
# localizator = localizator.Localizator(".localizator")
res_polygons_gui = []


def clear_res_polygons_gui():
    for gui_polygon in res_polygons_gui:
        gui.scene.removeItem(gui_polygon.polygon)
    res_polygons_gui.clear()


def set_up_scene():
    # localizator.stop()
    clear_res_polygons_gui()
    gui.clear_scene()
    scene_file = gui.get_field('scene')
    success = ps.load_scene(scene_file)
    loaded_scene = scene_file if success else None
    # localizator.run(loaded_scene)


def compute():
    clear_res_polygons_gui()

    d = gui.get_field('d')
    try:
        d = float(d)
    except:
        print("invalid d value")
        return
    print("compute", d)

    # with open("C:\\projects\\university\\algorithmic_robotics_and_motion_planning\\project\\src\\py\\.localizator\\8174572161177201748\\.outfile", "r") as outfile:
    #     data = json.load(outfile)
    # res = []
    # polygons_json = data["polygons"]
    # for polygon in polygons_json:
    #     res.append(np.array(polygon))
    # res = localizator.query1(float(d))
    res = []
    for polygon in res:
        fill_color = QtGui.QColor(0, 0, 255, 100)
        line_color = QtCore.Qt.transparent
        gui_polygon = gui.add_polygon(polygon, fill_color, line_color)
        res_polygons_gui.append(gui_polygon)


def enable():
    # gui.set_button_text('help', "Help")
    # gui.set_logic('help', display_help)
    # gui.set_label('mode', "Mode:")
    gui.set_label('scene', "Scene File (.json):")
    gui.set_logic('scene', set_up_scene)
    gui.set_button_text('scene', "Load Scene")
    # gui.set_label('planner', "Planner File (.py):")
    # gui.set_logic('planner', generate_path)
    # gui.set_button_text('planner', "Generate Path")
    gui.set_button_text('searchScene', "..")
    gui.set_logic('searchScene', set_input_file)
    # gui.set_button_text('searchPlanner', "..")
    gui.set_button_text('compute', "Compute")
    gui.set_logic('compute', compute)
    gui.set_label('d', "measurement size (d)")
    # gui.set_label('solution', "Solution File (.txt):")
    # gui.set_logic('solution', load_path)
    # gui.set_button_text('solution', "Load Solution")
    # gui.set_button_text('searchSolution', "..")
    # gui.set_logic('searchSolution', set_solution_file)
    # gui.set_label('export', "Export solution:")
    # gui.set_logic('export', export_path)
    # gui.set_button_text('export', "Export")
    # gui.set_button_text('animate', "Animate Movement Along Path")
    # gui.set_logic('animate', animate_path)
    # gui.set_button_text('verify', "Check Path Validity")
    # gui.set_logic('verify', is_path_valid)


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
