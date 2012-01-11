#!/usr/bin/env python

# Copyright Singapore-MIT Alliance for Research and Technology

from PyQt4 import QtGui, QtCore
from gzip import GzipFile
from bz2 import BZ2File

from session import Session
from static.network import Network
from dynamic.city import City
from controller import Controller
from animator import Animator
from scene import Graphics_scene
from view import Graphics_view

class Visualizer(QtGui.QMainWindow):
    def __init__(self, parent=None):
        super(Visualizer, self).__init__(parent)
        self.setWindowTitle("Stop and Go -- SimMobility Visualizer in PyQt4")

        self.controller = Controller(self)
        self.addDockWidget(QtCore.Qt.LeftDockWidgetArea, self.controller)

        self.view = Graphics_view(self)
        self.setCentralWidget(self.view)

    def load(self, file_name):
        self.scene = Graphics_scene(self)
        self.view.setScene(self.scene)

        self.session = Session()
        self.road_network = Network()
        self.animator = Animator(self.scene)
        self.city = City(self.animator)

        if file_name.endswith(".gz"):
            input_file = GzipFile(file_name)
        elif file_name.endswith(".bz2"):
            input_file = BZ2File(file_name)
        else:
            input_file = open(file_name, 'r')
        for line in input_file:
            if self.session.parse(line):
                continue
            if self.road_network.parse(line):
                continue
            if self.city.parse(line):
                continue
        input_file.close()

        self.road_network.resolve()
        self.scene.draw_road_network(self.road_network)
