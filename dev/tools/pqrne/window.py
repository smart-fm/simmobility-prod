#!/usr/bin/env python

# Copyright (2012) Singapore-MIT Alliance for Research and Technology

import sys
import elixir
from PyQt4 import QtGui, QtCore

from ui_window import Ui_main_window
from scene import Graphics_scene
from view import Graphics_view
from wanderlust import Wanderlust
from digital_map import Digital_map
from network import Road_network
from error import show_error_message

class Main_window(QtGui.QMainWindow, Ui_main_window):
    def __init__(self, parent=None):
        super(Main_window, self).__init__(parent)
        self.setupUi(self)

        self.selection_box_vertical_layout.setStretchFactor(self.selection_scroll_area, 1)

        self.wanderlust = Wanderlust(self)
        self.scene = Graphics_scene(self)
        self.view = Graphics_view(self.scene, self)
        self.setCentralWidget(self.view)

    def set_databases(self, shape_file_database, road_network_database):
        self.digital_map = Digital_map(shape_file_database)
        self.road_network = Road_network(road_network_database)

        elixir.setup_all()
        elixir.create_all()

        self.digital_map.load_database()
        self.road_network.load_database()

        self.scene.clear()
        self.scene.draw_digital_map(self.digital_map)
        self.scene.draw_road_network(self.road_network)

        self.wanderlust.load_digital_map(self.digital_map)
        self.wanderlust.load_road_network(self.road_network)

    def show_selected_road_items_info(self, road_info):
        self.selection_label.setText(road_info)

    def closeEvent(self, event):
        self.quit()

    def quit(self):
        if self.digital_map.is_dirty() or self.road_network.is_dirty():
            msg_box = QtGui.QMessageBox()
            msg_box.setWindowTitle("pqrne: quit message")
            msg_box.setText("The databases were modified")
            msg_box.setInformativeText("You have modified the databases.  "
                "Do you wish to save the changes to the databases?")
            msg_box.setStandardButtons(QtGui.QMessageBox.Save | QtGui.QMessageBox.Discard)
            msg_box.setDefaultButton(QtGui.QMessageBox.Save)
            reply = msg_box.exec_()
            if reply == QtGui.QMessageBox.Save:
                self.on_action_save_triggered(True)

    @QtCore.pyqtSignature("bool")
    def on_action_save_triggered(self, is_checked):
        if self.digital_map.is_dirty():
            self.digital_map.commit_to_database()
        if self.road_network.is_dirty():
            self.road_network.commit_to_database()

    @QtCore.pyqtSignature("bool")
    def on_action_exit_triggered(self, is_checked):
        self.quit()
        sys.exit(0)

    @QtCore.pyqtSignature("bool")
    def on_action_show_selection_box_triggered(self, is_checked):
        if is_checked and not self.selection_box.isVisible():
            self.selection_box.show()
        else:
            self.selection_box.hide()

    @QtCore.pyqtSignature("bool")
    def on_selection_box_visibilityChanged(self, is_visible):
        if not is_visible and self.action_show_selection_box.isChecked():
            self.action_show_selection_box.setChecked(False)

    @QtCore.pyqtSignature("bool")
    def on_action_show_polyline_dots_triggered(self, is_checked):
        if is_checked:
            self.scene.selector.show_dots()
        else:
            self.scene.selector.hide_dots()

    @QtCore.pyqtSignature("bool")
    def on_action_snap_to_line_triggered(self, is_checked):
        if is_checked:
            self.action_snap_to_line.setText("Snap to line")
            self.view.snap_to_line = True
        else:
            self.action_snap_to_line.setText("Snap to point")
            self.view.snap_to_line = False

    @QtCore.pyqtSignature("bool")
    def on_action_extract_crossings_and_stop_lines_triggered(self, is_checked):
        if self.view.selections.selection_region:
            path = self.view.selections.selection_region.path()
            self.view.selections.reset_selection_region()
        else:
            path = QtGui.QPainterPath()
            path.addRect(self.scene.sceneRect())
        lane_edges = self.wanderlust.extract_crossings_and_stop_lines(path, ["M"])
        self.wanderlust.extend_stop_lines_to_kerb(lane_edges)
        lane_edges = self.wanderlust.extract_crossings_and_stop_lines(path, ["J", "A4"])
        self.wanderlust.extend_stop_lines_to_kerb(lane_edges)

    @QtCore.pyqtSignature("bool")
    def on_action_extract_yellow_box_triggered(self, is_checked):
        if self.view.selections.selection_region:
            path = self.view.selections.selection_region.path()
            self.view.selections.reset_selection_region()
        else:
            path = QtGui.QPainterPath()
            path.addRect(self.scene.sceneRect())
        self.wanderlust.extract_yellow_boxes(path)

    @QtCore.pyqtSignature("bool")
    def on_action_create_missing_stop_line_triggered(self, is_checked):
        if not self.view.selections.selection_region:
            show_error_message("You must select a rectangular region which includes a crossing "
                "and the lane-markings that would end at the (missing) stop-line.  Use <Control> "
                "+ Right-mouse-button to create the selection region.")
            return
        path = self.view.selections.selection_region.path()
        self.view.selections.reset_selection_region()
        self.wanderlust.create_missing_stop_line(path)

    @QtCore.pyqtSignature("bool")
    def on_action_edit_selected_road_item_triggered(self, is_checked):
        self.view.edit_selected_items()

    @QtCore.pyqtSignature("bool")
    def on_action_delete_lane_edge_triggered(self, is_checked):
        self.view.delete_selected_item()

# vim:columns=100:smartindent:shiftwidth=4:expandtab:softtabstop=4:
