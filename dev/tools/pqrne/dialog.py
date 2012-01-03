#!/usr/bin/python

# Copyright Singapore-MIT Alliance for Research and Technology

from PyQt4 import QtGui, QtCore

class Dialogs:
    help_dialog = None
    lane_edge_dialog = None
    center_line_dialog = None

class Lane_edge_dialog(QtGui.QDialog):
    types = ("lane edge",
             "pedestrian crossing",
             "stop line",
             "kerb line",
             "yellow box",
             "bus zone",
             "zebra crossing")
    type_letters = ("e",
                    "J",
                    "M",
                    "k",
                    "N",
                    "R",
                    "z")

    def __init__(self, parent=None):
        super(Lane_edge_dialog, self).__init__(parent)
        self.setWindowTitle("pqrne edit lane-edge information")

        section_id_label = QtGui.QLabel("Section-id:")
        self.section_id_line_edit = QtGui.QLineEdit()
        section_id_label.setBuddy(self.section_id_line_edit)

        edge_index_label = QtGui.QLabel("Edge index:")
        self.edge_index_spin_box = QtGui.QSpinBox()
        edge_index_label.setBuddy(self.edge_index_spin_box)

        type_label = QtGui.QLabel("Type:")
        self.type_combo_box = QtGui.QComboBox()
        type_label.setBuddy(self.type_combo_box)
        self.type_combo_box.addItems(Lane_edge_dialog.types)

        polyline_label = QtGui.QLabel("Polyline:")
        self.polyline_tree_widget = QtGui.QTreeWidget()
        header = self.polyline_tree_widget.header()
        header.setResizeMode(0, QtGui.QHeaderView.ResizeToContents)
        polyline_label.setBuddy(self.polyline_tree_widget)
        self.polyline_tree_widget.setAlternatingRowColors(True)
        self.polyline_tree_widget.setIndentation(0)
        self.polyline_tree_widget.setHeaderLabels(["#", "x", "y"])

        layout = QtGui.QGridLayout()
        layout.addWidget(section_id_label, 0, 0)
        layout.addWidget(self.section_id_line_edit, 0, 1)
        layout.addWidget(edge_index_label, 1, 0)
        layout.addWidget(self.edge_index_spin_box, 1, 1)
        layout.addWidget(type_label, 2, 0)
        layout.addWidget(self.type_combo_box, 2, 1)
        layout.addWidget(polyline_label, 3, 0, 1, 2)
        layout.addWidget(self.polyline_tree_widget, 4, 0, 1, 2)

        button_box = QtGui.QDialogButtonBox(QtGui.QDialogButtonBox.Ok | QtGui.QDialogButtonBox.Cancel)
        button_box.button(QtGui.QDialogButtonBox.Ok).setDefault(True)
        layout.addWidget(button_box, 5, 0, 1, 2)

        self.setLayout(layout)

        self.connect(button_box, QtCore.SIGNAL("accepted()"), self, QtCore.SLOT("accept()"))
        self.connect(button_box, QtCore.SIGNAL("rejected()"), self, QtCore.SLOT("reject()"))

    def set_lane_edge(self, lane_edge):
        self.section_id_line_edit.setText(str(lane_edge.section_id))
        self.edge_index_spin_box.setValue(lane_edge.index)
        try:
            i = Lane_edge_dialog.type_letters.index(lane_edge.type)
            self.type_combo_box.setCurrentIndex(i)
        except ValueError:
            self.type_combo_box.setCurrentIndex(0)
        self.polyline_tree_widget.clear()
        for i, point in enumerate(lane_edge.polyline):
            row = QtGui.QTreeWidgetItem([str(i), str(point.x), str(point.y)])
            self.polyline_tree_widget.addTopLevelItem(row)

    def section_id(self):
        return int(self.section_id_line_edit.text())

    def edge_index(self):
        return self.edge_index_spin_box.value()

    def edge_type(self):
        return Lane_edge_dialog.type_letters[self.type_combo_box.currentIndex()]

    def update(self, lane_edge):
        lane_edge.section_id = self.section_id()
        lane_edge.index = self.edge_index()
        lane_edge.type = self.edge_type()

class Center_line_dialog(QtGui.QDialog):
    def __init__(self, parent=None):
        super(Center_line_dialog, self).__init__(parent)
        self.setWindowTitle("pqrne edit center-line information")

        section_id_label = QtGui.QLabel("Section-id:")
        self.section_id_line_edit = QtGui.QLineEdit()
        section_id_label.setBuddy(self.section_id_line_edit)

        lane_index_label = QtGui.QLabel("Lane index:")
        self.lane_index_spin_box = QtGui.QSpinBox()
        lane_index_label.setBuddy(self.lane_index_spin_box)

        type_label = QtGui.QLabel("Type:")
        type_value_label = QtGui.QLabel("lane center line")

        self.can_go_straight_check_box = QtGui.QCheckBox("Can go straight")
        self.can_turn_left_check_box = QtGui.QCheckBox("Can turn left")
        self.can_turn_right_check_box = QtGui.QCheckBox("Can turn right")
        self.can_turn_on_red_signal_check_box = QtGui.QCheckBox("Can turn on red signal")
        self.can_change_lane_left_check_box = QtGui.QCheckBox("Can change lane left")
        self.can_change_lane_right_check_box = QtGui.QCheckBox("Can change lane right")
        self.is_road_shoulder_check_box = QtGui.QCheckBox("Is road shoulder")
        self.is_bicycle_lane_check_box = QtGui.QCheckBox("Is bicycle lane")
        self.is_pedestrian_lane_check_box = QtGui.QCheckBox("Is pedestrian lane")
        self.is_vehicle_lane_check_box = QtGui.QCheckBox("Is vehicle lane")
        self.is_standard_bus_lane_check_box = QtGui.QCheckBox("Is standard bus lane")
        self.is_whole_day_bus_lane_check_box = QtGui.QCheckBox("Is whole day bus lane")
        self.is_high_occupancy_vehicle_lane_check_box = QtGui.QCheckBox("Is high-occupancy-vehicle lane")
        self.can_freely_park_here_check_box = QtGui.QCheckBox("Can freely park here")
        self.can_stop_here_check_box = QtGui.QCheckBox("Can stop here")
        self.is_U_turn_allowed_here_check_box = QtGui.QCheckBox("Is U-turn allowed here")

        polyline_label = QtGui.QLabel("Polyline:")
        self.polyline_tree_widget = QtGui.QTreeWidget()
        header = self.polyline_tree_widget.header()
        header.setResizeMode(0, QtGui.QHeaderView.ResizeToContents)
        polyline_label.setBuddy(self.polyline_tree_widget)
        self.polyline_tree_widget.setAlternatingRowColors(True)
        self.polyline_tree_widget.setIndentation(0)
        self.polyline_tree_widget.setHeaderLabels(["#", "x", "y"])

        layout = QtGui.QGridLayout()
        layout.addWidget(section_id_label, 0, 0)
        layout.addWidget(self.section_id_line_edit, 0, 1)
        layout.addWidget(lane_index_label, 1, 0)
        layout.addWidget(self.lane_index_spin_box, 1, 1)
        layout.addWidget(type_label, 2, 0)
        layout.addWidget(type_value_label, 2, 1)

        vbox = QtGui.QVBoxLayout()
        vbox.addWidget(self.can_go_straight_check_box)
        vbox.addWidget(self.can_turn_left_check_box)
        vbox.addWidget(self.can_turn_right_check_box)
        vbox.addWidget(self.can_turn_on_red_signal_check_box) 
        vbox.addWidget(self.can_change_lane_left_check_box)
        vbox.addWidget(self.can_change_lane_right_check_box)
        vbox.addWidget(self.is_road_shoulder_check_box)
        vbox.addWidget(self.is_bicycle_lane_check_box)
        vbox.addWidget(self.is_pedestrian_lane_check_box)
        vbox.addWidget(self.is_vehicle_lane_check_box)
        vbox.addWidget(self.is_standard_bus_lane_check_box)
        vbox.addWidget(self.is_whole_day_bus_lane_check_box)
        vbox.addWidget(self.is_high_occupancy_vehicle_lane_check_box)
        vbox.addWidget(self.can_freely_park_here_check_box)
        vbox.addWidget(self.can_stop_here_check_box)
        vbox.addWidget(self.is_U_turn_allowed_here_check_box)
        layout.addLayout(vbox, 3, 0)

        layout.addWidget(polyline_label, 4, 0, 1, 2)
        layout.addWidget(self.polyline_tree_widget, 5, 0, 1, 2)

        button_box = QtGui.QDialogButtonBox(QtGui.QDialogButtonBox.Ok | QtGui.QDialogButtonBox.Cancel)
        button_box.button(QtGui.QDialogButtonBox.Ok).setDefault(True)
        layout.addWidget(button_box, 6, 0, 1, 2)

        self.setLayout(layout)

        self.connect(button_box, QtCore.SIGNAL("accepted()"), self, QtCore.SLOT("accept()"))
        self.connect(button_box, QtCore.SIGNAL("rejected()"), self, QtCore.SLOT("reject()"))

    def set_center_line(self, center_line):
        self.section_id_line_edit.setText(str(center_line.section_id))
        self.lane_index_spin_box.setValue(center_line.index)

        self.can_go_straight_check_box.setChecked(center_line.can_go_straight)
        self.can_turn_left_check_box.setChecked(center_line.can_turn_left)
        self.can_turn_right_check_box.setChecked(center_line.can_turn_right)
        self.can_turn_on_red_signal_check_box.setChecked(center_line.can_turn_on_red_signal)
        self.can_change_lane_left_check_box.setChecked(center_line.can_change_lane_left)
        self.can_change_lane_right_check_box.setChecked(center_line.can_change_lane_right)
        self.is_road_shoulder_check_box.setChecked(center_line.is_road_shoulder)
        self.is_bicycle_lane_check_box.setChecked(center_line.is_bicycle_lane)
        self.is_pedestrian_lane_check_box.setChecked(center_line.is_pedestrian_lane)
        self.is_vehicle_lane_check_box.setChecked(center_line.is_vehicle_lane)
        self.is_standard_bus_lane_check_box.setChecked(center_line.is_standard_bus_lane)
        self.is_whole_day_bus_lane_check_box.setChecked(center_line.is_whole_day_bus_lane)
        self.is_high_occupancy_vehicle_lane_check_box.setChecked(center_line.is_high_occupancy_vehicle_lane)
        self.can_freely_park_here_check_box.setChecked(center_line.can_freely_park_here)
        self.can_stop_here_check_box.setChecked(center_line.can_stop_here)
        self.is_U_turn_allowed_here_check_box.setChecked(center_line.is_U_turn_allowed_here)

        self.polyline_tree_widget.clear()
        for i, point in enumerate(center_line.polyline):
            row = QtGui.QTreeWidgetItem([str(i), str(point.x), str(point.y)])
            self.polyline_tree_widget.addTopLevelItem(row)

    def update(self, center_line):
        center_line.section_id = int(self.section_id_line_edit.text())
        center_line.index = self.lane_index_spin_box.value()

        center_line.can_go_straight = self.can_go_straight_check_box.isChecked()
        center_line.can_turn_left = self.can_turn_left_check_box.isChecked()
        center_line.can_turn_right = self.can_turn_right_check_box.isChecked()
        center_line.can_turn_on_red_signal = self.can_turn_on_red_signal_check_box.isChecked()
        center_line.can_change_lane_left = self.can_change_lane_left_check_box.isChecked()
        center_line.can_change_lane_right = self.can_change_lane_right_check_box.isChecked()
        center_line.is_road_shoulder = self.is_road_shoulder_check_box.isChecked()
        center_line.is_bicycle_lane = self.is_bicycle_lane_check_box.isChecked()
        center_line.is_pedestrian_lane = self.is_pedestrian_lane_check_box.isChecked()
        center_line.is_vehicle_lane = self.is_vehicle_lane_check_box.isChecked()
        center_line.is_standard_bus_lane = self.is_standard_bus_lane_check_box.isChecked()
        center_line.is_whole_day_bus_lane = self.is_whole_day_bus_lane_check_box.isChecked()
        center_line.is_high_occupancy_vehicle_lane = self.is_high_occupancy_vehicle_lane_check_box.isChecked()
        center_line.can_freely_park_here = self.can_freely_park_here_check_box.isChecked()
        center_line.can_stop_here = self.can_stop_here_check_box.isChecked()
        center_line.is_U_turn_allowed_here = self.is_U_turn_allowed_here_check_box.isChecked()
