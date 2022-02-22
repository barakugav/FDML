from PyQt5 import QtCore, QtGui, QtWidgets
from PyQt5.QtWidgets import (QApplication, QGraphicsView,
                             QGraphicsPixmapItem, QGraphicsScene, QGraphicsPolygonItem,
                             QGraphicsEllipseItem, QGraphicsLineItem, QOpenGLWidget)
from PyQt5.QtGui import QPainter, QPixmap, QPolygonF, QPen
from PyQt5.QtCore import (QObject, QPointF, QPoint, QRectF,
                          QPropertyAnimation, pyqtProperty, QSequentialAnimationGroup,
                          QParallelAnimationGroup, QPauseAnimation, Qt)


class RPolygonWithHoles(QObject):
    """A class that represents a polygon with holes on screen (as a Qt5 object)

    :param points: list of points of the polygon
    :type points: list<tuple>
    :param holes: list of holes, each hole is a list of points
    :type holes: list<list<tuple>>
    :param line_color: color of the boundary of polygon
    :type line_color: class:`QtGui.QColor`
    :param fill_color: color of the interior of polygon
    :type fill_color: class:`QtGui.QColor`
    :param line_width: width of the boundary of polygon
    :type line_width: int
    """
    def __init__(self, points, holes, line_color, fill_color, line_width):
        self._points = [QPointF(p[0], p[1]) for p in points]
        self._holes = [[QPointF(p[0], p[1]) for p in hole] for hole in holes]
        self._pos = self._points[0]
        self.fill_color = fill_color
        super().__init__()
        # The underlying QGraphicsPolygonItem

        self.polygon = QGraphicsPolygonItem()
        poly = QPolygonF(self._points)
        for hole in self._holes:
            poly = poly.subtracted(QPolygonF(hole))
        self.polygon.setPolygon(poly)
        self.polygon.setBrush(QtGui.QBrush(fill_color))
        pen = QPen(QtGui.QPen(line_color))
        pen.setWidthF(line_width)
        self.polygon.setPen(pen)
        self._visible = 1


    def x(self):
        """
        Return x position of the first vertex in polygon

        :return: x position of the first vertex in polygon
        :rtype: float
        """
        return self._pos.x()


    def y(self):
        """
        Return y position of the first vertex in polygon

        :return: y position of the first vertex in polygon
        :rtype: float
        """
        return self._pos.y()


    def points(self):
        """
        Return a list of polygon's vertices
        
        :return: list polygon vertices
        :rtype: list<QPointF>
        """
        return self._points


    ####################################################
    # The following functions are for animation support
    ####################################################


    @pyqtProperty(QPointF)
    def pos(self):
        """
        Return the position of the first vertex of polygon
        (Required for animation support by Qt5)

        :return: position of the first vertex of polygon
        :rtype: QPointF
        """
        return self._pos


    @pos.setter
    def pos(self, value):
        """
        Shift the entire polygon such that the first vertex is in
        the new coordinate
        (Required for animation support by Qt5)

        :param value: new position of the first polygon vertex
        :type value: QPointF
        """
        delta_x = value.x() - self._pos.x()
        delta_y = value.y() - self._pos.y()
        self._points = [QPointF(p.x() + delta_x, p.y() + delta_y) for p in self._points]
        self.polygon.setPolygon(QPolygonF(self._points))
        self._pos = value


    @pyqtProperty(int)
    def visible(self):
        """
        Get the visibility of the polygon
        (Required for animation support by Qt5)

        :return: visibility of the polygon
        :rtype: int
        """
        return self._visible


    @visible.setter
    def visible(self, value):
        """
        Set the visibility of the polygon
        (Required for animation support by Qt5)

        :param value: new visibility of the polygon
        :type value: int
        """
        if value > 0:
            self.polygon.show()
        else:
            self.polygon.hide()
        self._visible = value
