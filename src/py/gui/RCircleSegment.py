from PyQt5 import QtCore, QtGui, QtWidgets
from PyQt5.QtWidgets import (QApplication, QGraphicsView,
                             QGraphicsPixmapItem, QGraphicsScene, QGraphicsPolygonItem,
                             QGraphicsEllipseItem, QGraphicsLineItem, QGraphicsPathItem, QOpenGLWidget)
from PyQt5.QtGui import QPainter, QPixmap, QPolygonF, QPen, QPainterPath
from PyQt5.QtCore import (QObject, QPointF, QPoint, QRectF,
                          QPropertyAnimation, pyqtProperty, QSequentialAnimationGroup,
                          QParallelAnimationGroup, QPauseAnimation, Qt)
import math


class RCircleSegment(QObject):
    """A class that represents a circle segment on screen (as a Qt5 object)
    A circle segment is a non-linear connected curve that is contained in some circle
    In this representation, it is made of a circle plus start and end angles

    :param radius: radius of circle
    :type radius: float
    :param center_x: x position of circle center
    :type center_x: float
    :param center_y: y position of circle center
    :type center_y: float
    :param start_angle: start angle of circle segment
    :type start_angle: float
    :param end_angle: end angle of circle segment
    :type end_angle: float
    :param clockwise: draw from start to end clockwise or counter-clockwise
    :type clockwise: bool
    :param line_width: width of the boundary of the circle segment
    :type line_width: int
    :param line_color: color of the circle segment
    :type line_color: class:`QtGui.QColor`
    :param fill_color: color of the interior of circle
    :type fill_color: class:`QtGui.QColor`
    """
    def __init__(self,radius: float, center_x: float, center_y: float, start_angle: float, 
            end_angle: float, clockwise, line_width: float, line_color = Qt.black, fill_color=Qt.transparent):
        super().__init__()
        # The supporting rectangle
        if end_angle < start_angle:
            end_angle += 2*math.pi
        start_angle = -start_angle
        end_angle = -end_angle
        shift = end_angle-start_angle
        if clockwise:
           shift = -shift-2*math.pi
        x, y = center_x - radius, center_y - radius
        self.rect = QRectF(x, y, 2 * radius, 2 * radius)
        # The underlying QGraphicsPathItem
        self.painter_path = QPainterPath(QPointF(center_x+math.cos(start_angle)*radius, center_y-math.sin(start_angle)*radius))
        self.painter_path.arcTo(self.rect, math.degrees(start_angle), math.degrees(shift))
        self.path = QGraphicsPathItem(self.painter_path)
        self.path.setBrush(QtGui.QBrush(fill_color))
        pen = QPen()
        pen.setWidthF(line_width)
        pen.setColor(line_color)
        self.path.setPen(pen)
        self._visible = 1


    ####################################################
    # The following functions are for animation support
    ####################################################


    @pyqtProperty(int)
    def visible(self):
        """
        Get the visibility of the circle segment
        (Required for animation support by Qt5)

        :param value: visibility of the circle segment
        :type value: int
        """
        return self._visible

    @visible.setter
    def visible(self, value):
        """
        Set the visibility of the circle segment
        (Required for animation support by Qt5)

        :return: visibility of the circle segment
        :rtype: int
        """
        if (value > 0):
            self.path.show()
        else:
            self.path.hide()
        self._visible = value
