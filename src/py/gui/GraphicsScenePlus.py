from PyQt5 import QtWidgets
from PyQt5.QtCore import pyqtSignal, Qt


class GraphicsScenePlus(QtWidgets.QGraphicsScene):
    """
    Extender for Qt's QGraphicsScene
    Add support for general useful capabilities which are not supported
    natively for QGraphicsScene

    Adding signals for mouse (left and right) clicks, and scroll for zoom in/out
    """
    def __init__(self, gui):
        super().__init__()
        self.gui = gui


    def wheelEvent(self, event):
        """
        Handle mouse wheel events

        :param event: mouse wheel event
        :type event: QEvent
        """
        angle = event.delta() / 8
        if angle > 0:  # scroll up
            self.gui.zoom /= 0.9
        else:  # scroll down
            self.gui.zoom *= 0.9
        self.gui.redraw()
        event.accept()


    def mousePressEvent(self, event):
        """
        Handle mouse (left or right) press events

        :param event: mouse press event
        :type event: QEvent
        """
        x, y = event.scenePos().x(), event.scenePos().y()
        if event.button() == Qt.RightButton:
            # if right button clicked emit right button signal
            self.right_click_signal.emit(x, y)
        elif event.button() == Qt.LeftButton:
            # if left button clicked emit left button signal
            self.left_click_signal.emit(x, y)
        super(GraphicsScenePlus, self).mousePressEvent(event)

    #: Signal when user right mouse clicks the graphics view
    right_click_signal = pyqtSignal(float, float)
    #: Signal when user left mouse clicks the graphics view
    left_click_signal = pyqtSignal(float, float)