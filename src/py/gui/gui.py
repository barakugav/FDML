from PyQt5 import QtCore, QtGui, QtWidgets
#from PyQt5.QtOpenGL import QGLWidget, QGLFormat, QGL
from PyQt5.QtWidgets import (QApplication, QGraphicsView,
                             QGraphicsPixmapItem, QGraphicsScene, QGraphicsPolygonItem,
                             QGraphicsEllipseItem, QGraphicsLineItem, QGraphicsTextItem, QOpenGLWidget)
from PyQt5.QtGui import QPainter, QPixmap, QPolygonF, QPen, QVector3D, QPalette, QFont
from PyQt5.QtCore import (QObject, QPointF, QPoint, QRectF,
                          QPropertyAnimation, pyqtProperty, QSequentialAnimationGroup,
                          QParallelAnimationGroup, QPauseAnimation, Qt, pyqtSignal)


from MainWindowsPlus import MainWindowPlus
from GraphicsScenePlus import GraphicsScenePlus

from RPolygon import RPolygon
from RPolygonWithHoles import RPolygonWithHoles
from RDisc import RDisc
from RSegment import RSegment
from RText import RText
from RCircleSegment import RCircleSegment
from RSegment_angle import RSegment_angle

import time
import math


class GUI(object):
    """
    The RMP GUI class, handles all the boilerplate code for having GUI capable Qt projects

    Any GUI application should derive from this class, and set somewhere in the layout the
    property `graphics_view` to a QGraphicsView.
    The function you should override in this case is setup_ui()
    """
    def __init__(self):
        self.mainWindow = MainWindowPlus(self)
        self.scene = GraphicsScenePlus(self)
        self.sequence = QSequentialAnimationGroup()
        self.sequence.finished.connect(self.animation_finished)
        self.setup_ui()
        self._setup_view()


    #: Screen width
    width = 1600
    #: Screen height
    height = 1000
    #: View zoom
    zoom = 1.0
    #: Base (default) line width
    base_line_width = 3.5
    #: Base (default) text size
    base_text_size = 2 * zoom
    #: Function reference that is called when animation is finished
    animation_finished_action = lambda: None

    #: Dictionary of all UI line inputs
    lineEdits = dict()
    #: Dictionary of all UI push buttons
    pushButtons = dict()
    #: Dictionary of all UI labels
    labels = dict()
    #: Dictionary of all UI progress bars
    progressBars = dict()

    #: GraphicsScenePlus scene that contains all the objects
    scene = None
    #: MainWindowPlus main window
    mainWindow = None
    #: QSequentialAnimationGroup sequence of animation
    sequence = None
    #: QGraphicsView widget where everything is drawn
    graphics_view = None


    def setup_ui(self):
        """
        Setup the UI layout of the application.
        Should be overrided by any GUI application and define a custom layout.
        """
        pass


    def _setup_view(self):
        """
        Setup and initialize the graphics view
        """
        if self.graphics_view is not None:
            self.graphics_view.setScene(self.scene)
            self.graphics_view.setSceneRect(0, 0, 0, 0)
            self.graphics_view.setRenderHints(QPainter.Antialiasing)
            # self.graphics_view.fitInView(self.scene.sceneRect(), Qt.KeepAspectRatio)

            # Enable for smoother animations and Antialiasing - may cause some issues with multiple displays
            # self.graphics_view.setViewport(QGLWidget(QGLFormat(QGL.SampleBuffers)))

            self.graphics_view.scale(self.zoom, -self.zoom)
            self.graphics_view.setDragMode(1)


    def add_disc(self, r, x, y, fill_color=QtCore.Qt.black, line_color=QtCore.Qt.black):
        """
        Add a disc to the scene with radius r centered at (x, y) and return the object associated with it

        :param r: radius of disc
        :type r: float
        :param x: x position of center of disc
        :type x: float
        :param y: y position of center of disc
        :type y: float
        :param fill_color: color of the interior of disc
        :type fill_color: class:`QtGui.QColor`
        :param line_color: color of the boundary of disc
        :type line_color: class:`QtGui.QColor`

        :return: disc
        :rtype: RDisc
        """
        d = RDisc(r, x, y, fill_color, line_color=line_color, line_width=self.base_line_width / self.zoom)
        self.scene.addItem(d.disc)
        return d


    def add_polygon(self, points, fill_color=QtCore.Qt.black, line_color=QtCore.Qt.black):
        """
        Add a polygon to the scene and return the object associated with it

        :param points: list of points of polygon
        :type points: list<tuple>
        :param fill_color: color of the interior of polygon
        :type fill_color: class:`QtGui.QColor`
        :param line_color: color of the boundary of polygon
        :type line_color: class:`QtGui.QColor`

        :return: polygon
        :rtype: RPolygon
        """
        p = RPolygon(points, fill_color=fill_color, line_color=line_color, line_width=self.base_line_width / self.zoom)
        self.scene.addItem(p.polygon)
        return p


    def add_polygon_with_holes(self, points, holes, fill_color=QtCore.Qt.black, line_color=QtCore.Qt.transparent):
        """
        Add a polygon with holes to the scene and return the object associated with it

        :param points: list of points of polygon
        :type points: list<tuple>
        :param holes: list of holes, each hole is a list of points
        :type holes: list<list<tuple>>
        :param fill_color: color of the interior of polygon
        :type fill_color: class:`QtGui.QColor`
        :param line_color: color of the boundary of polygon
        :type line_color: class:`QtGui.QColor`

        :return: polygon
        :rtype: RPolygonWithHoles
        """
        p = RPolygonWithHoles(points, holes, line_color, fill_color, line_width=self.base_line_width / self.zoom)
        self.scene.addItem(p.polygon)
        return p


    def add_segment(self, x1, y1, x2, y2, line_color=QtCore.Qt.black, opacity=1.0):
        """
        Add a segment to the scene and return the object associated with it

        :param x1: x1 position of start endpoint
        :type x1: float
        :param y1: y1 position of start endpoint
        :type y1: float
        :param x2: x2 position of end endpoint
        :type x2: float
        :param y2: y2 position of end endpoint
        :type y2: float
        :param line_color: color of the segment
        :type line_color: class:`QtGui.QColor`
        :param opacity: opacity of the segment
        :type opacity: float

        :return: segment
        :rtype: RSegment
        """
        s = RSegment(x1, y1, x2, y2, color=line_color, line_width=self.base_line_width / self.zoom, opacity=opacity)
        self.scene.addItem(s.line)
        return s


    def add_segment_angle(self, x1, y1, length, angle, line_color=QtCore.Qt.black):
        """
        Add an angled segment to the scene and return the object associated with it

        :param x1: x1 position of start endpoint
        :type x1: float
        :param y1: y1 position of start endpoint
        :type y1: float
        :param length: length of segment
        :type length: float
        :param angle: angle (in radians) of segment
        :type angle: float
        :param line_color: color of the segment
        :type line_color: class:`QtGui.QColor`

        :return: angled segment
        :rtype: RSegment_angle
        """
        s = RSegment_angle(x1, y1, length, angle, line_color=line_color, line_width=self.base_line_width / self.zoom)
        self.scene.addItem(s.line)
        return s


    def add_circle_segment(self, radius: float, center_x: float, center_y: float, start_angle: float,
                           end_angle: float,
                           clockwise, fill_color=QtCore.Qt.transparent, line_color=QtCore.Qt.black):
        """
        Add a circle segment to the scene and return the object associated with it

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
        :param fill_color: color of the interior of circle
        :type fill_color: class:`QtGui.QColor`
        :param line_color: color of the circle segment
        :type line_color: class:`QtGui.QColor`

        :return: circle segment
        :rtype: RCircleSegment
        """
        s = RCircleSegment(radius, center_x, center_y, start_angle, end_angle,
                           line_width=self.base_line_width / self.zoom, clockwise=clockwise,
                           fill_color=fill_color, line_color=line_color)
        self.scene.addItem(s.path)
        return s


    def add_text(self, text, x, y, size, color=QtCore.Qt.black):
        """
        Add a text label to the scene and return the object associated with it

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

        :return: text label
        :rtype: RText
        """
        t = RText(text, x, y, size, color)
        self.scene.addItem(t.text)
        return t


    # Create a new linear translation animation for obj starting at ix, iy and ending at x, y
    def linear_translation_animation(self, obj, ix, iy, x, y, duration=1000):
        """
        Create a new linear translation animation for obj starting at ix, iy and ending at x, y

        :param obj: object to animate
        :type obj: QObject
        :param ix: x position of start
        :type ix: float
        :param iy: y position of start
        :type iy: float
        :param x: x position of end
        :type x: float
        :param y: y position of end
        :type y: float
        :param duration: duration of animation
        :type duration: int

        :return: animation object
        :rtype: QPropertyAnimation
        """
        anim = QPropertyAnimation(obj, b'pos')
        anim.setDuration(duration)
        anim.setStartValue(QPointF(ix, iy))
        anim.setEndValue(QPointF(x, y))
        return anim


    def segment_angle_animation(self, obj, ix, iy, ia, x, y, a, clockwise, duration=2000):
        """
        Create an animation for angle segment starting at ix, iy with angle ia ending at x, y, angle a

        :param obj: object to animate
        :type obj: QObject
        :param ix: x position of start segment
        :type ix: float
        :param iy: y position of start segment
        :type iy: float
        :param ia: rotation of the start segment
        :type ia: float
        :param x: x position of end segment
        :type x: float
        :param y: y position of end segment
        :type y: float
        :param a: rotation of the end segment
        :type a: float
        :param clockwise: rotate the segment clocwise
        :type clockwise: bool
        :param duration: duration of animation
        :type duration: int

        :return: animation object
        :rtype: QPropertyAnimation
        """
        if (ia > 2 * math.pi or ia < 0 or a > 2 * math.pi or a < 0):
            print("invalid angle values - must be >=0 and < 2*pi")
            a, ia = 0, 0
        anim = QPropertyAnimation(obj, b'pos')
        anim.setDuration(duration)
        r = 0
        if (not clockwise and a < ia): ia -= 2 * math.pi
        if (clockwise and a > ia): ia += 2 * math.pi
        start = QVector3D(float(ix), float(iy), float(ia))
        anim.setStartValue(start)
        end = QVector3D(float(x), float(y), float(a))
        anim.setEndValue(end)
        # anim.setKeyValueAt(0.999, QVector3D(float(x), float(y), float(a + r)))
        return anim


    def translation_animation(self, obj, func, duration=1000):
        """
        Create a general translation animation for obj. func is path from the unit interval I to R^2

        :param obj: object to animate
        :type obj: QObject
        :param func: a functino representing a path from unit interal I to R^2
        :type func: function<I->R^2>
        :param duration: duration of animation
        :type duration: int

        :return: animation object
        :rtype: QPropertyAnimation
        """
        anim = QPropertyAnimation(obj, b'pos')
        anim.setDuration(duration)
        anim.setStartValue(QPointF(func(0)[0], func(0)[1]))
        anim.setEndValue(QPointF(func(1)[0], func(1)[1]))
        vals = [p / 100 for p in range(0, 101)]
        for i in vals:
            anim.setKeyValueAt(i, (QPointF(func(i)[0], func(i)[1])))
        return anim


    def visibility_animation(self, obj, visible):
        """
        Create an animation the changes the visibility of an object

        :param obj: object to animate
        :type obj: QObject
        :param visible: the visibility we wat to set
        :type visible: bool

        :return: animation object
        :rtype: QPropertyAnimation
        """
        anim = QPropertyAnimation(obj, b'visible')
        anim.setDuration(0)
        if visible:
            anim.setEndValue(1)
        else:
            anim.setEndValue(0)
        return anim


    def pause_animation(self, duration=1000):
        """
        Create an animation that does nothing

        :param duration: duration of animation
        :type duration: int

        :return: animation object
        :rtype: QPropertyAnimation
        """
        anim = QPauseAnimation(duration)
        return anim


    def value_animation(self, obj, v_begin, v_end, duration=1000):
        """
        Create an animation that changes the value of an object

        :param obj: object to animate
        :type obj: QObject
        :param v_begin: the start value of the object
        :type v_begin: object
        :param v_end: the end value of the object
        :type v_end: object
        :param duration: duration of animation
        :type duration: int

        :return: animation object
        :rtype: QPropertyAnimation
        """
        anim = QPropertyAnimation(obj, b'value')
        anim.setDuration(duration)
        anim.setStartValue(v_begin)
        anim.setEndValue(v_end)
        return anim


    def text_animation(self, obj, text: int):
        """
        Create an animation that changes the text of an object

        :param obj: object to animate
        :type obj: QObject
        :param text: the new label we want to set
        :type text: int

        :return: animation object
        :rtype: QPropertyAnimation
        """
        anim = QPropertyAnimation(obj, b'text')
        anim.setDuration(0)
        anim.setEndValue(text)
        return anim


    def parallel_animation(self, *animations):
        """
        Create an animation from a set of animations that will run in parallel

        :param *animations: the animations we want to run in parallel
        :type *animations: QPropertyAnimation, QPropertyAnimation, ...

        :return: parallel animation group object
        :rtype: QParallelAnimationGroup
        """
        group = QParallelAnimationGroup()
        for anim in animations:
            group.addAnimation(anim)
        return group


    def queue_animation(self, *animations):
        """
        Add an animation to the animation queue

        :param *animations: the animations we want to add to queue
        :type *animations: QPropertyAnimation, QPropertyAnimation, ...
        """
        for anim in animations:
            self.sequence.addAnimation(anim)


    def play_queue(self):
        """
        Play (and empty) the animation queue
        """
        self.sequence.start()


    def stop_queue(self):
        """
        Stop (and empty) the animation queue
        """
        self.sequence.stop()
        self.animation_finished()


    def empty_queue(self):
        """
        Empty the animation queue
        """
        self.sequence.clear()


    def clear_scene(self):
        """
        Clear the scene of all objects
        """
        self.scene.clear()


    def redraw(self):
        """
        Redraw the scene with updated parameters
        """
        line_width = self.base_line_width / self.zoom
        text_size = max(1, self.base_text_size / self.zoom)
        for item in self.graphics_view.items():
            if not isinstance(item, QGraphicsTextItem):
                pen = item.pen()
                pen.setWidthF(line_width)
                item.setPen(pen)
            else:
                item.setFont(QFont("Times", text_size))
        self.graphics_view.resetTransform()
        self.graphics_view.scale(self.zoom, -self.zoom)


    def animation_finished(self):
        """
        Function that is called when the `finished` signal is fired
        Empties the queue and calls the finished action defined by the program
        """
        self.empty_queue()
        print("Finished playing animation")
        self.animation_finished_action()


    def set_animation_finished_action(self, action):
        """
        Set the function to be called when the animation finishes playing

        :param action: action to be called when animation finished
        :type action: function<()->()>
        """
        self.animation_finished_action = action


    def set_field(self, key, s):
        """
        Set the text of field with key <key> in the GUI

        :param key: key of line edit
        :type key: str
        :param s: new text for line edit
        :type s: str
        """
        self.lineEdits[key].setText(s)


    def get_field(self, key):
        """
        Get the text of field with key <key> in the GUI

        :param key: key of line edit
        :type key: str

        :return: text of selected line edit
        :rtype: str
        """
        return self.lineEdits[key].text()


    def set_label(self, key, s, color=Qt.black):
        """
        Set the text of label with key <key> in the GUI

        :param key: key of label
        :type key: str
        :param s: new label text
        :type s: str
        :param color: color of label
        :type color: QColor
        """
        self.labels[key].setText(s)
        palette = self.labels[key].palette()
        palette.setColor(QPalette.WindowText, color)
        self.labels[key].setPalette(palette)


    def set_logic(self, key, logic):
        """
        Set the function to be called when the button with key <key> in the GUI is pressed

        :param key: key of pushbutton
        :type key: str
        :param logic: function that is connected to the button
        :type logic: function<()->()>
        """
        try:
            self.pushButtons[key].clicked.disconnect()
        except Exception:
            pass
        self.pushButtons[key].clicked.connect(logic)


    def set_button_text(self, key, s):
        """
        Set the text of the button with key <key> in the GUI

        :param key: key of the pushbutton
        :type key: str
        :param s: new label of pushbutton
        :type s: str
        """
        self.pushButtons[key].setText(s)


    def set_progressbar_value(self, key, n: int):
        """
        Set the value of the progressBar with key <key>

        :param key: key of the progress bar
        :type key: str
        :param n: value of the progress bar
        :type n: int
        """
        self.progressBars[key].setValue(n)


    def set_program_name(self, s):
        """
        Set the program's name (title of the window)

        :param s: new window title
        :type s: str
        """
        self.mainWindow.setWindowTitle(s)
