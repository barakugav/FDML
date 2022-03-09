from PyQt5.QtCore import pyqtSignal, QObject
from PyQt5.QtGui import QBrush, QColor, QTextCursor, QTextCharFormat
from PyQt5.QtWidgets import QTextEdit


class Logger(QTextEdit):
    """
    Widget class that represents a GUI log window in the app
    You can append text to the log in different colors
    """
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
    

    def set_color(self, color):
        """
        Change the drawing color of the text

        :param color: new color
        :type color: QColor
        """
        self.color = QBrush(QColor(color))

    def append_text(self, string):
        """
        Append text the the bottom of the log.
        The appended text's color will be the one selected in the
        `color` brush member of the widget

        :param string: new text to append
        :type string: str
        """
        super().moveCursor(QTextCursor.End)
        cursor = QTextCursor(super().textCursor())

        format_ = QTextCharFormat()
        format_.setForeground(QBrush(QColor(self.color)))
        cursor.setCharFormat(format_)
        cursor.insertText(string)


    #: color of the text that will be drawn next
    color = QBrush(QColor('black'))


class Writer(QObject):
    """
    A QObject that handles writing to the logger widget

    :param logger: logger widget reference
    :type logger: Logger
    """
    def __init__(self, logger: Logger):
        super(Writer, self).__init__()
        self.text_edit = logger
        self.text_edit.setReadOnly(True)
        self.append_text_signal.connect(logger.append_text)
        self.change_color_signal.connect(logger.set_color)


    def write(self, string):
        """
        Write text into the logger, and emit a signal

        :param string: new text to append
        :type string: str
        """
        print(string, end='')
        self.append_text_signal.emit(string)


    def set_color(self, color):
        """
        Set the brush color of the logger, and emit a signal

        :param color: brush color of the logger
        :type color: QColor
        """
        self.change_color_signal.emit(color)


    #: Signal that is fired when text is appended to the logger
    append_text_signal = pyqtSignal(str)
    #: Signal that is fired when brush color is changed
    change_color_signal = pyqtSignal(str)
