from PyQt5 import QtCore, QtGui, QtWidgets
from PyQt5.QtWidgets import (QApplication, QGraphicsView,
                             QGraphicsPixmapItem, QGraphicsScene, QGraphicsPolygonItem,
                             QGraphicsEllipseItem, QGraphicsLineItem, QOpenGLWidget)
from PyQt5.QtGui import QPainter, QPixmap, QPolygonF, QPen, QVector3D
from PyQt5.QtCore import (QObject, QPointF, QPoint, QRectF,
                          QPropertyAnimation, pyqtProperty, QSequentialAnimationGroup,
                          QParallelAnimationGroup, QPauseAnimation, Qt)
import math


class RSegment_angle(QObject):
    """A class that represents an angled linear segment on screen (as a Qt5 object)
    The segment is defined with its start position, its length and the angle
    (Instead of start and end position like in RSegment)

    :param x1: x position of start endpoint
    :type x1: float
    :param y1: y position of start endpoint
    :type y1: float
    :param l: length of the segment
    :type l: float
    :param a: angle (in radians) af the segment
    :type a: float
    :param line_color: color of the segment
    :type line_color: class:`QtGui.QColor`
    :param line_width: width of segment
    :type line_width: int
    """
    def __init__(self, x1, y1, l, a, line_color, line_width):
        self._point_radius = 3
        self._angle = a
        self._length = l
        self._x1 = x1
        self._y1 = y1
        self._x2 = x1 + self._length * math.cos(self._angle)
        self._y2 = y1 + self._length * math.sin(self._angle)
        self._pos = QVector3D(x1, y1, self._angle)
        super().__init__()
        self.line = QGraphicsLineItem()
        self.rect = QRectF(x1 - self._point_radius, y1 - self._point_radius, 2 * self._point_radius,
                           2 * self._point_radius)
        self.point = QGraphicsEllipseItem()
        self.point.setRect(self.rect)
        self.line.setLine(self._x1, self._y1, self._x2, self._y2)
        pen = QPen()
        pen.setWidthF(line_width)
        pen.setColor(line_color)
        self.line.setPen(pen)
        self.point.setPen(pen)


    def x(self):
        """
        Return x position of the start endpoint of segment

        :return: x position of the start endpoint of segment
        :rtype: float
        """
        return self._x1


    def y(self):
        """
        Return y position of the start endpoint of segment

        :return: y position of the start endpoint of segment
        :rtype: float
        """
        return self._y1


    def angle(self):
        """
        Return the angle of the segment

        :return: angle of segment
        :rtype: float
        """
        return self._angle


    ####################################################
    # The following functions are for animation support
    ####################################################


    @pyqtProperty(QVector3D)
    def pos(self):
        """
        Return the representation of the position of segment
        The segment is represented as (x, y, angle) - a 3D vector
        (Required for animation support by Qt5)

        :return: position representation of the segment
        :rtype: QVector3D
        """
        return self._pos


    @pos.setter
    def pos(self, value):
        """
        Transform and rotate the segment to a new position and rotation
        (Required for animation support by Qt5)

        :param value: new representation for angled segment
        :type value: QVector3D
        """
        self._pos = value
        self._x1 = self._pos.x()
        self._y1 = self._pos.y()
        self._angle = value.z()
        self._x2 = self._x1 + self._length * math.cos(self._angle)
        self._y2 = self._y1 + self._length * math.sin(self._angle)
        self.line.setLine(self._x1, self._y1, self._x2, self._y2)
        self.rect = QRectF(self._x1 - self._point_radius, self._y1 - self._point_radius, 2 * self._point_radius,
                           2 * self._point_radius)
        self.point.setRect(self.rect)
