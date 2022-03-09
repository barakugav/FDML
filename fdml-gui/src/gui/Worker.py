from PyQt5 import QtCore
from PyQt5.QtCore import pyqtSignal, QObject


class WorkerSignals(QObject):
    """
    Struct of all signals required by a Worker
    """

    #: Signal that is fired when worker task is finished, has also the task output
    finished = pyqtSignal(list)


class Worker(QtCore.QRunnable):
    """
    Task worker that can run a function in parallel to GUI.
    Function's result is returned in the `finished` signal

    :param fn: function that the worker runs
    :type fn: function(...->object)
    :param *args: arguments to be passed to function
    :type *args: object, object, ...
    """
    def __init__(self, fn, *args):
        super(Worker, self).__init__()

        # Store constructor arguments (re-used for processing)
        self.fn = fn
        self.args = args
        self.isRunning = [True]
        self.signals = WorkerSignals()


    def stop(self):
        """
        Stop the current task
        """
        self.isRunning[0] = False


    @QtCore.pyqtSlot()
    def run(self):
        """
        Initialise the runner function with passed args, kwargs.
        """
        # Retrieve args/kwargs here; and fire processing using them
        res = []
        res.append(self.fn(*self.args, self.isRunning))
        self.signals.finished.emit(res)  # Done
