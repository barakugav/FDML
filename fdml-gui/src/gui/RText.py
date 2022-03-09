from PyQt5 import QtCore, QtGui, QtWidgets
from PyQt5.QtWidgets import (QApplication, QGraphicsView,
                             QGraphicsPixmapItem, QGraphicsScene, QGraphicsPolygonItem,
                             QGraphicsEllipseItem, QGraphicsLineItem, QGraphicsTextItem, QOpenGLWidget)
from PyQt5.QtGui import QPainter, QPixmap, QPolygonF, QPen, QFont, QTransform
from PyQt5.QtCore import (QObject, QPointF, QPoint, QRectF,
                          QPropertyAnimation, pyqtProperty, QSequentialAnimationGroup,
                          QParallelAnimationGroup, QPauseAnimation, Qt)


class RText(QObject):
    """A class that represents a text label on screen (as a Qt5 object)

    :param text: text label
    :type text: str
    :param x: x position of text label
    :type x: float
    :param y: y position of text label
    :type y: float
    :param size: size of label
    :type size: int
    :param color: color of the label
    :type color: class:`QtGui.QColor`
    """
    def __init__(self, text, x, y, size, color):
        self._pos = QPointF(x - 1.8, y + 1.8)
        super().__init__()
        self.text = QGraphicsTextItem(text)
        transform = QTransform.fromScale(0.3, -0.3)
        self.text.setTransformOriginPoint(self._pos)
        self.text.setTransform(transform)
        self.text.setPos(self._pos)
        # self.text.setRotation(-180)
        font = QFont("Times", 2)
        self.text.setFont(font)

        self._visible = 1


    ####################################################
    # The following functions are for animation support
    ####################################################


    @pyqtProperty(int)
    def visible(self):
        """
        Get the visibility of the label
        (Required for animation support by Qt5)

        :param value: visibility of the label
        :type value: int
        """
        return self._visible


    @visible.setter
    def visible(self, value):
        """
        Set the visibility of the label
        (Required for animation support by Qt5)

        :return: visibility of the label
        :rtype: int
        """
        if value > 0:
            self.text.show()
        else:
            self.text.hide()
        self._visible = value

