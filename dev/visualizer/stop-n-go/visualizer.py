#!/usr/bin/env python

# Copyright Singapore-MIT Alliance for Research and Technology

from PyQt4 import QtGui, QtCore

from view import Graphics_view
from controller import Controller

class Visualizer(QtGui.QMainWindow):
    def __init__(self, parent=None):
        super(Visualizer, self).__init__(parent)
        self.setWindowTitle("Stop and Go -- SimMobility Visualizer in PyQt4")

        self.graphics_view = Graphics_view(self)
        self.graphics_view.setMinimumSize(640, 480)
        self.setCentralWidget(self.graphics_view)

        self.controller = Controller(self, self.graphics_view, self.statusBar())
        self.addDockWidget(QtCore.Qt.LeftDockWidgetArea, self.controller)
