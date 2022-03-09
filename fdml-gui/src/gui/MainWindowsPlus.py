from PyQt5 import QtCore, QtWidgets
from PyQt5.QtCore import Qt, pyqtSignal


class MainWindowPlus(QtWidgets.QMainWindow):
    """
    Extender for Qt's QMainWindow
    Add support for general useful capabilities which are not supported
    natively for QMainWindow

    Adding file dragging capability and +/- keys for zoom in/out
    """
    def __init__(self, gui):
        super().__init__()
        self.gui = gui
        self.setAcceptDrops(True)


    def keyPressEvent(self, event):
        """
        Handle key press events

        :param event: key press event
        :type event: QEvent
        """
        # Adjust zoom level/scale on +/- key press
        # https://doc.qt.io/qt-5/qt.html#Key-enum
        if event.key() == QtCore.Qt.Key_Plus:
            self.gui.zoom /= 0.9
        if event.key() == QtCore.Qt.Key_Minus:
            self.gui.zoom *= 0.9
        self.gui.redraw()

    
    def dragEnterEvent(self, event):
        """
        Handle drag enter events

        :param event: drag enter event
        :type event: QEvent
        """
        # do nothing
        event.accept()


    def dragMoveEvent(self, event):
        """
        Handle drag move events

        :param event: drag move event
        :type event: QEvent
        """
        # do nothing
        event.accept()

    def dropEvent(self, event):
        """
        Handle drog events

        :param event: drop event
        :type event: QEvent
        """
        # If user dropped a file, emit a signal with the dropped filename
        if event.mimeData().hasText:
            event.setDropAction(Qt.CopyAction)
            file_path: str = event.mimeData().urls()[0].toLocalFile()
            self.signal_drop.emit(file_path)
        event.accept()

    #: Signal when user drags a file to the app
    signal_drop = pyqtSignal(str)
