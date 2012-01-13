#!/usr/bin/env python

# Copyright Singapore-MIT Alliance for Research and Technology

from PyQt4 import QtGui, QtCore

import ui_controller

class Controller(QtGui.QDockWidget,
                 ui_controller.Ui_controller):
    def __init__(self, parent=None):
        super(Controller, self).__init__(parent)
        self.setupUi(self)

        self.visualizer = parent
        self.interval = 100

    @QtCore.pyqtSignature("int")
    def on_frame_number_spin_box_valueChanged(self, frame_number):
        pass

    @QtCore.pyqtSignature("")
    def on_load_file_push_button_clicked(self):
        file_name = QtGui.QFileDialog.getOpenFileName(self.visualizer, "Choose file to load", ".")
        if len(file_name):
            self.log_file_name_line_edit.setText(file_name)
            self.visualizer.load(str(file_name))

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
        self.visualizer.animator.go_to_frame_number(frame_number)

        if self.is_running:
            self.frame_number_spin_box.stepUp()
            QtCore.QTimer.singleShot(self.interval, self.run)
