from PyQt5 import QtCore, QtGui, QtWidgets
from PyQt5.QtCore import pyqtSignal
from gui import GUI, MainWindowPlus


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

    def setupUi(self):
        self.mainWindow = MainWindowSceneDesigner(self)
        MainWindow = self.mainWindow

        MainWindow.setStyleSheet("QMainWindow { background-color : rgb(54, 57, 63); color : rgb(220, 221, 222); }\n"
                                 "#centralwidget { background-color : rgb(54, 57, 63); color : rgb(220, 221, 222); }\n"
                                 "QLabel { background-color : rgb(54, 57, 63); color : rgb(220, 221, 222); }")
        self.centralwidget = QtWidgets.QWidget(MainWindow)
        self.centralwidget.setObjectName("centralwidget")
        self.gridLayout = QtWidgets.QGridLayout(self.centralwidget)
        self.gridLayout.setObjectName("gridLayout")
        self.graphicsView = QtWidgets.QGraphicsView(self.centralwidget)
        self.graphicsView.setEnabled(True)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Expanding)
        sizePolicy.setHorizontalStretch(1)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.graphicsView.sizePolicy().hasHeightForWidth())
        self.graphicsView.setSizePolicy(sizePolicy)
        self.graphicsView.setObjectName("graphicsView")
        self.gridLayout.addWidget(self.graphicsView, 3, 1, 1, 1)
        self.gridLayout_0 = QtWidgets.QGridLayout()
        self.gridLayout_0.setObjectName("gridLayout_0")
        self.pushButton_3 = QtWidgets.QPushButton(self.centralwidget)
        font = QtGui.QFont()
        font.setPointSize(12)
        self.pushButton_3.setFont(font)
        self.pushButton_3.setObjectName("pushButton_3")
        self.gridLayout_0.addWidget(self.pushButton_3, 15, 0, 1, 1)
        # self.lineEdit_7 = QtWidgets.QLineEdit(self.centralwidget)
        # font = QtGui.QFont()
        # font.setPointSize(12)
        # self.lineEdit_7.setFont(font)
        # self.lineEdit_7.setObjectName("lineEdit_7")
        # self.gridLayout_0.addWidget(self.lineEdit_7, 24, 0, 1, 1)
        # self.pushButton_4 = QtWidgets.QPushButton(self.centralwidget)
        # font = QtGui.QFont()
        # font.setPointSize(12)
        # self.pushButton_4.setFont(font)
        # self.pushButton_4.setObjectName("pushButton_4")
        # self.gridLayout_0.addWidget(self.pushButton_4, 22, 0, 1, 1)
        spacerItem = QtWidgets.QSpacerItem(300, 20, QtWidgets.QSizePolicy.MinimumExpanding,
                                           QtWidgets.QSizePolicy.Minimum)
        self.gridLayout_0.addItem(spacerItem, 33, 0, 1, 1)
        self.label_1 = QtWidgets.QLabel(self.centralwidget)
        font = QtGui.QFont()
        font.setPointSize(12)
        self.label_1.setFont(font)
        self.label_1.setObjectName("label_1")
        self.gridLayout_0.addWidget(self.label_1, 4, 0, 1, 1)
        self.lineEdit_0 = QtWidgets.QLineEdit(self.centralwidget)
        font = QtGui.QFont()
        font.setPointSize(12)
        self.lineEdit_0.setFont(font)
        self.lineEdit_0.setObjectName("lineEdit_0")
        self.gridLayout_0.addWidget(self.lineEdit_0, 2, 0, 1, 1)
        self.lineEdit_1 = QtWidgets.QLineEdit(self.centralwidget)
        font = QtGui.QFont()
        font.setPointSize(12)
        self.lineEdit_1.setFont(font)
        self.lineEdit_1.setObjectName("lineEdit_1")
        self.gridLayout_0.addWidget(self.lineEdit_1, 5, 0, 1, 1)
        # self.lineEdit_4 = QtWidgets.QLineEdit(self.centralwidget)
        # font = QtGui.QFont()
        # font.setPointSize(12)
        # self.lineEdit_4.setFont(font)
        # self.lineEdit_4.setObjectName("lineEdit_4")
        # self.gridLayout_0.addWidget(self.lineEdit_4, 17, 0, 1, 1)
        self.label_0 = QtWidgets.QLabel(self.centralwidget)
        font = QtGui.QFont()
        font.setPointSize(12)
        self.label_0.setFont(font)
        self.label_0.setObjectName("label_0")
        self.gridLayout_0.addWidget(self.label_0, 1, 0, 1, 1)
        # self.lineEdit_2 = QtWidgets.QLineEdit(self.centralwidget)
        # font = QtGui.QFont()
        # font.setPointSize(12)
        # self.lineEdit_2.setFont(font)
        # self.lineEdit_2.setObjectName("lineEdit_2")
        # self.gridLayout_0.addWidget(self.lineEdit_2, 11, 0, 1, 1)
        # self.label_6 = QtWidgets.QLabel(self.centralwidget)
        # font = QtGui.QFont()
        # font.setPointSize(12)
        # self.label_6.setFont(font)
        # self.label_6.setObjectName("label_6")
        # self.gridLayout_0.addWidget(self.label_6, 20, 0, 1, 1)
        # self.label_7 = QtWidgets.QLabel(self.centralwidget)
        # font = QtGui.QFont()
        # font.setPointSize(12)
        # self.label_7.setFont(font)
        # self.label_7.setObjectName("label_7")
        # self.gridLayout_0.addWidget(self.label_7, 23, 0, 1, 1)
        self.textEdit = QtWidgets.QTextEdit(self.centralwidget)
        self.textEdit.setObjectName("textEdit")
        self.gridLayout_0.addWidget(self.textEdit, 9, 0, 1, 1)
        # self.lineEdit_5 = QtWidgets.QLineEdit(self.centralwidget)
        # font = QtGui.QFont()
        # font.setPointSize(12)
        # self.lineEdit_5.setFont(font)
        # self.lineEdit_5.setObjectName("lineEdit_5")
        # self.gridLayout_0.addWidget(self.lineEdit_5, 19, 0, 1, 1)
        # self.pushButton_5 = QtWidgets.QPushButton(self.centralwidget)
        # font = QtGui.QFont()
        # font.setPointSize(12)
        # self.pushButton_5.setFont(font)
        # self.pushButton_5.setObjectName("pushButton_5")
        # self.gridLayout_0.addWidget(self.pushButton_5, 0, 0, 1, 1)
        self.pushButton_1 = QtWidgets.QPushButton(self.centralwidget)
        font = QtGui.QFont()
        font.setPointSize(12)
        self.pushButton_1.setFont(font)
        self.pushButton_1.setObjectName("pushButton_1")
        self.gridLayout_0.addWidget(self.pushButton_1, 8, 0, 1, 1)
        # self.pushButton_6 = QtWidgets.QPushButton(self.centralwidget)
        # font = QtGui.QFont()
        # font.setPointSize(12)
        # self.pushButton_6.setFont(font)
        # self.pushButton_6.setObjectName("pushButton_6")
        # self.gridLayout_0.addWidget(self.pushButton_6, 28, 0, 1, 1)
        # self.label_2 = QtWidgets.QLabel(self.centralwidget)
        # font = QtGui.QFont()
        # font.setPointSize(12)
        # self.label_2.setFont(font)
        # self.label_2.setObjectName("label_2")
        # self.gridLayout_0.addWidget(self.label_2, 10, 0, 1, 1)
        # self.lineEdit_6 = QtWidgets.QLineEdit(self.centralwidget)
        # font = QtGui.QFont()
        # font.setPointSize(12)
        # self.lineEdit_6.setFont(font)
        # self.lineEdit_6.setObjectName("lineEdit_6")
        # self.gridLayout_0.addWidget(self.lineEdit_6, 21, 0, 1, 1)
        self.label_3 = QtWidgets.QLabel(self.centralwidget)
        font = QtGui.QFont()
        font.setPointSize(12)
        self.label_3.setFont(font)
        self.label_3.setObjectName("label_3")
        self.gridLayout_0.addWidget(self.label_3, 13, 0, 1, 1)
        # self.label_4 = QtWidgets.QLabel(self.centralwidget)
        # font = QtGui.QFont()
        # font.setPointSize(12)
        # self.label_4.setFont(font)
        # self.label_4.setObjectName("label_4")
        # self.gridLayout_0.addWidget(self.label_4, 16, 0, 1, 1)
        self.lineEdit_3 = QtWidgets.QLineEdit(self.centralwidget)
        font = QtGui.QFont()
        font.setPointSize(12)
        self.lineEdit_3.setFont(font)
        self.lineEdit_3.setObjectName("lineEdit_3")
        self.gridLayout_0.addWidget(self.lineEdit_3, 14, 0, 1, 1)
        # self.pushButton_2 = QtWidgets.QPushButton(self.centralwidget)
        # font = QtGui.QFont()
        # font.setPointSize(12)
        # self.pushButton_2.setFont(font)
        # self.pushButton_2.setObjectName("pushButton_2")
        # self.gridLayout_0.addWidget(self.pushButton_2, 12, 0, 1, 1)
        self.toolButton_0 = QtWidgets.QToolButton(self.centralwidget)
        font = QtGui.QFont()
        font.setPointSize(12)
        self.toolButton_0.setFont(font)
        self.toolButton_0.setObjectName("toolButton_0")
        self.gridLayout_0.addWidget(self.toolButton_0, 2, 1, 1, 1)
        # self.label_5 = QtWidgets.QLabel(self.centralwidget)
        # font = QtGui.QFont()
        # font.setPointSize(12)
        # self.label_5.setFont(font)
        # self.label_5.setObjectName("label_5")
        # self.gridLayout_0.addWidget(self.label_5, 18, 0, 1, 1)
        self.pushButton_7 = QtWidgets.QPushButton(self.centralwidget)
        font = QtGui.QFont()
        font.setPointSize(12)
        self.pushButton_7.setFont(font)
        self.pushButton_7.setObjectName("pushButton_7")
        self.gridLayout_0.addWidget(self.pushButton_7, 29, 0, 1, 1)
        spacerItem1 = QtWidgets.QSpacerItem(20, 40, QtWidgets.QSizePolicy.Minimum, QtWidgets.QSizePolicy.Expanding)
        self.gridLayout_0.addItem(spacerItem1, 30, 0, 1, 1)
        self.pushButton_0 = QtWidgets.QPushButton(self.centralwidget)
        font = QtGui.QFont()
        font.setPointSize(12)
        self.pushButton_0.setFont(font)
        self.pushButton_0.setObjectName("pushButton_0")
        self.gridLayout_0.addWidget(self.pushButton_0, 3, 0, 1, 1)
        self.gridLayout.addLayout(self.gridLayout_0, 3, 0, 1, 1)
        MainWindow.setCentralWidget(self.centralwidget)
        self.statusbar = QtWidgets.QStatusBar(MainWindow)
        self.statusbar.setObjectName("statusbar")
        MainWindow.setStatusBar(self.statusbar)

        QtCore.QMetaObject.connectSlotsByName(MainWindow)

        # end of eq designer generated code

        self.lineEdits['scene'] = self.lineEdit_0
        self.lineEdits['saveLocation'] = self.lineEdit_1
        # self.lineEdits['index'] = self.lineEdit_2
        self.lineEdits['resolution'] = self.lineEdit_3
        # self.lineEdits['innerRadius'] = self.lineEdit_4
        # self.lineEdits['blockNumber'] = self.lineEdit_5
        # self.lineEdits['blockHeight'] = self.lineEdit_6
        # self.lineEdits['radius'] = self.lineEdit_7
        self.pushButtons['load']= self.pushButton_0
        self.pushButtons['save'] = self.pushButton_1
        # self.pushButtons['select'] = self.pushButton_2
        self.pushButtons['resolution'] = self.pushButton_3
        # self.pushButtons['addCircularRoom'] = self.pushButton_4
        # self.pushButtons['help'] = self.pushButton_5
        # self.pushButtons['radius'] = self.pushButton_6
        self.pushButtons['clear'] = self.pushButton_7
        self.pushButtons['searchScene'] = self.toolButton_0
        self.labels['scene'] = self.label_0
        self.labels['saveLocation'] = self.label_1
        # self.labels['index'] = self.label_2
        self.labels['resolution'] = self.label_3
        # self.labels['innerRadius'] = self.label_4
        # self.labels['blockNumber'] = self.label_5
        # self.labels['blockHeight'] = self.label_6
        # self.labels['radius'] = self.label_7
