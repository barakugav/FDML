from PyQt5 import QtCore, QtGui, QtWidgets
from PyQt5.QtWidgets import (QApplication, QGraphicsView,
                             QGraphicsPixmapItem, QGraphicsScene, QGraphicsPolygonItem,
                             QGraphicsEllipseItem, QGraphicsLineItem, QOpenGLWidget)
from PyQt5.QtGui import QPainter, QPixmap, QPolygonF, QPen
from PyQt5.QtCore import (QObject, QPointF, QPoint, QRectF,
                          QPropertyAnimation, pyqtProperty, QSequentialAnimationGroup,
                          QParallelAnimationGroup, QPauseAnimation, Qt)


class RSegment(QObject):
    """A class that represents a linear segment on screen (as a Qt5 object)
    The segment is defined with its start and end positions of endpoints
    (Instead of start position, length and angle like in RSegment_angle)

    :param x1: x position of start endpoint
    :type x1: float
    :param y1: y position of start endpoint
    :type y1: float
    :param x2: x position of end endpoint
    :type x2: float
    :param y2: y position of end endpoint
    :type y2: float
    :param color: color of the segment
    :type color: class:`QtGui.QColor`
    :param line_width: width of segment
    :type line_width: int
    """
    def __init__(self, x1, y1, x2, y2, color, line_width, opacity):
        self._x1 = x1
        self._y1 = y1
        self._x2 = x2
        self._y2 = y2
        self._pos = QPointF(x1, y1)
        super().__init__()
        self.line = QGraphicsLineItem()
        self.line.setOpacity(opacity)
        self.line.setLine(x1, y1, x2, y2)
        pen = QPen()
        pen.setWidthF(line_width)
        pen.setColor(color)
        self.line.setPen(pen)


    def x(self):
        """
        Return x position of the start endpoint of segment

        :return: x position of the start endpoint of segment
        :rtype: float
        """
        return self._pos.x()


    def y(self):
        """
        Return y position of the start endpoint of segment

        :return: y position of the start endpoint of segment
        :rtype: float
        """
        return self._pos.y()


    ####################################################
    # The following functions are for animation support
    ####################################################


    @pyqtProperty(QPointF)
    def pos(self):
        """
        Return the position of start endpoint of segment
        (Required for animation support by Qt5)

        :return: position of start endpoint of segment
        :rtype: QPointF
        """
        return self._pos


    @pos.setter
    def pos(self, value):
        """
        Transform the segment to a new position
        (Required for animation support by Qt5)

        :param value: new position of start endpoint of segment
        :type value: QPointF
        """
        delta_x = value.x() - self._pos.x()
        delta_y = value.y() - self._pos.y()
        self._x1 = self._x1 + delta_x
        self._y1 = self._y1 + delta_y
        self._x2 = self._x2 + delta_x
        self._y2 = self._y2 + delta_y
        self.line.setLine(self._x1, self._y1, self._x2, self._y2)
        self._pos = value
