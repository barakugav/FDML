from PyQt5 import QtCore, QtGui, QtWidgets
from PyQt5.QtWidgets import (QApplication, QGraphicsView,
                             QGraphicsPixmapItem, QGraphicsScene, QGraphicsPolygonItem,
                             QGraphicsEllipseItem, QGraphicsLineItem, QOpenGLWidget)
from PyQt5.QtGui import QPainter, QPixmap, QPolygonF, QPen
from PyQt5.QtCore import (QObject, QPointF, QPoint, QRectF,
                          QPropertyAnimation, pyqtProperty, QSequentialAnimationGroup,
                          QParallelAnimationGroup, QPauseAnimation, Qt)


class RDisc(QObject):
    """A class that represents a disc on screen (as a Qt5 object)

    :param r: radius of disc
    :type r: float
    :param x: x location of disc
    :type x: float
    :param y: y location of disc
    :type y: float
    :param color: color of the interior of disc
    :type color: class:`QtGui.QColor`
    :param line_color: color of the boundary of disc
    :type line_color: class:`QtGui.QColor`
    :param line_width: width of the boundary of disc
    :type line_width: int
    """
    def __init__(self, r, x, y, color, line_color, line_width):
        self._radius = r
        self._pos = QPointF(x, y)
        super().__init__()
        # The supporting rectangle
        self.rect = QRectF(x - r, y - r, 2 * r, 2 * r)
        # The underlying QGraphicsEllipseItem
        self.disc = QGraphicsEllipseItem()
        self.disc.setRect(self.rect)
        self.disc.setBrush(QtGui.QBrush(color))
        pen = QPen()
        pen.setWidthF(line_width)
        pen.setColor(line_color)
        self.disc.setPen(pen)
        self._visible = 1


    def x(self):
        """
        Return x position of the center of disc

        :return: x position of the center of disc
        :rtype: float
        """
        return self._pos.x()


    def y(self):
        """
        Return y position of the center of disc

        :return: y position of the center of disc
        :rtype: float
        """
        return self._pos.y()


    ####################################################
    # The following functions are for animation support
    ####################################################


    @pyqtProperty(QPointF)
    def pos(self):
        """
        Return the position of the disc
        (Required for animation support by Qt5)

        :return: position of the disc
        :rtype: QPointF
        """
        return self._pos


    @pos.setter
    def pos(self, value):
        """
        Set the position of the disc
        (Required for animation support by Qt5)

        :param value: new position of the disc
        :type value: QPointF
        """
        self.rect = QRectF(value.x() - self._radius, value.y() - self._radius, 2 * self._radius, 2 * self._radius)
        self.disc.setRect(self.rect)
        self._pos = value


    @pyqtProperty(int)
    def visible(self):
        """
        Get the visibility of the disc
        (Required for animation support by Qt5)

        :return: visibility of the disc
        :rtype: int
        """
        return self._visible


    @visible.setter
    def visible(self, value):
        """
        Set the visibility of the disc
        (Required for animation support by Qt5)

        :param value: new visibility of the disc
        :type value: int
        """
        if (value > 0):
            self.disc.show()
        else:
            self.disc.hide()
        self._visible = value
