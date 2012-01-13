#!/usr/bin/env python

# Copyright Singapore-MIT Alliance for Research and Technology

from PyQt4 import QtGui, QtCore
from gzip import GzipFile
from bz2 import BZ2File

import ui_controller
from session import Session
from static.network import Network
from dynamic.city import City
from animator import Animator
from scene import Graphics_scene

class Controller(QtGui.QDockWidget,
                 ui_controller.Ui_controller):
    def __init__(self, parent, visualizer_view, status_bar):
        super(Controller, self).__init__(parent)
        self.parent = parent
        self.setupUi(self)

        self.visualizer_view = visualizer_view
        self.status_bar = status_bar
        self.interval = 100

    @QtCore.pyqtSignature("int")
    def on_frame_number_spin_box_valueChanged(self, frame_number):
        pass

    @QtCore.pyqtSignature("")
    def on_load_file_push_button_clicked(self):
        file_name = QtGui.QFileDialog.getOpenFileName(self.parent, "Choose file to load", ".")
        if len(file_name):
            self.load_log_file(str(file_name))

    def load_log_file(self, file_name):
        self.scene = Graphics_scene(self)
        self.visualizer_view.setScene(self.scene)

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

        self.log_file_name_line_edit.setText(file_name)

    @QtCore.pyqtSignature("bool")
    def on_stop_or_go_push_button_toggled(self, state):
        button = self.stop_or_go_push_button
        if button.isChecked():
            button.setText("Stop")
            self.frame_number_spin_box.setEnabled(False)
            self.frame_number_spin_box.setReadOnly(True)
            self.is_running = True
            QtCore.QTimer.singleShot(self.interval, self.run)
        else:
            button.setText("Go")
            self.is_running = False
            self.frame_number_spin_box.setEnabled(True)
            self.frame_number_spin_box.setReadOnly(False)

    def run(self):
        frame_number = self.frame_number_spin_box.value()
        self.animator.go_to_frame_number(frame_number)

        if self.is_running:
            self.frame_number_spin_box.stepUp()
            QtCore.QTimer.singleShot(self.interval, self.run)
