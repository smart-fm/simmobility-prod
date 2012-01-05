#!/usr/bin/env python

# Copyright Singapore-MIT Alliance for Research and Technology

from PyQt4 import QtGui, QtCore

import scene
import view
import tool
from dialog import Dialogs, Lane_edge_dialog, Center_line_dialog, Output_xml_dialog
import help

class Main_window(QtGui.QMainWindow):
    def __init__(self, road_network, parent=None):
        super(Main_window, self).__init__(parent)
        self.setWindowTitle("SimMobility Road Network Editor in PyQt4")
        self.road_network = road_network

        self.tool_box = tool.Tool_box(self)
        self.addDockWidget(QtCore.Qt.LeftDockWidgetArea, self.tool_box)

        self.scene = scene.Graphics_scene(self)
        self.scene.draw_road_network(road_network, self.tool_box)

        self.view = view.Graphics_view(self, road_network)
        self.view.setScene(self.scene)
        self.setCentralWidget(self.view)

        self.connect(self.tool_box.point_or_line_button, QtCore.SIGNAL("toggled(bool)"),
                     self.view.toggle_point_or_line)
        self.connect(self.tool_box.help_button, QtCore.SIGNAL("clicked()"), self.show_help)

        Dialogs.help_dialog = help.create_manual()
        Dialogs.lane_edge_dialog = Lane_edge_dialog(self)
        Dialogs.center_line_dialog = Center_line_dialog(self)
        Dialogs.output_xml_dialog = Output_xml_dialog(self)

    def closeEvent(self, event):
        msg_box = QtGui.QMessageBox()
        #msg_box.setText("You may or may not have make any changes.")
        msg_box.setInformativeText("Do you want to save your changes?")
        msg_box.setStandardButtons(QtGui.QMessageBox.Save | QtGui.QMessageBox.Discard)
        msg_box.setDefaultButton(QtGui.QMessageBox.Save)
        reply = msg_box.exec_()
        if QtGui.QMessageBox.Save == reply:
            self.road_network.save()

    def show_help(self):
        Dialogs.help_dialog.open()
