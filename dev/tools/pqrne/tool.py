#!/usr/bin/env python

# Copyright Singapore-MIT Alliance for Research and Technology

from PyQt4 import QtGui, QtCore

class Tool_box(QtGui.QDockWidget):
    VISIBILITY_BUTTON_COLUMN = 1
    SELECTABILITY_BUTTON_COLUMN = 2

    def __init__(self, parent=None):
        super(Tool_box, self).__init__("pqrne tool box", parent)
        self.setFeatures(self.features() & ~QtGui.QDockWidget.DockWidgetClosable)
        self.setMinimumWidth(350)

        widget = QtGui.QWidget()
        self.setWidget(widget)
        layout = QtGui.QVBoxLayout()
        widget.setLayout(layout)
        self.help_button = QtGui.QPushButton("Show manual")
        layout.addWidget(self.help_button)
        self.make_point_or_line_button()
        layout.addWidget(self.point_or_line_button)
        layout.addWidget(self.make_checkables())

    def make_checkables(self):
        tree = QtGui.QTreeWidget()
        tree.setHeaderHidden(True)
        tree.setColumnCount(3)
        self.checkables_tree = tree

        self.lane_marking_group = self.add_group("Lane markings")
        self.kerb_line_group = self.add_group("Kerb lines")
        self.lane_edge_group = self.add_group("Lane Edges")

        return tree

    def add_group(self, group_name):
        row = QtGui.QTreeWidgetItem([group_name, "visibility", "selectability"])
        self.checkables_tree.addTopLevelItem(row)
        self.make_check_boxes(row, True)
        row.types = dict()
        return row

    def lane_marking_type(self, type):
        if type not in self.lane_marking_group.types:
            child = QtGui.QTreeWidgetItem([type, "visibility", "selectability"])
            child.items = list()
            self.lane_marking_group.addChild(child)
            self.lane_marking_group.types[type] = child
            self.make_check_boxes(child)
        return self.lane_marking_group.types[type]

    def kerb_line_type(self, type):
        if type not in self.kerb_line_group.types:
            child = QtGui.QTreeWidgetItem([type, "visibility", "selectability"])
            child.items = list()
            self.kerb_line_group.addChild(child)
            self.kerb_line_group.types[type] = child
            self.make_check_boxes(child)
        return self.kerb_line_group.types[type]

    def lane_edge_type(self, type):
        if type not in self.lane_edge_group.types:
            child = QtGui.QTreeWidgetItem([type, "visibility", "selectability"])
            child.items = list()
            self.lane_edge_group.addChild(child)
            self.lane_edge_group.types[type] = child
            self.make_check_boxes(child)
        return self.lane_edge_group.types[type]

    def make_check_boxes(self, row, is_top_level=False):
        button = QtGui.QCheckBox("visible")
        button.setAutoFillBackground(True)
        button.setCheckState(QtCore.Qt.Checked)
        self.checkables_tree.setItemWidget(row, Tool_box.VISIBILITY_BUTTON_COLUMN, button)
        if not is_top_level:
            button.items = row.items
            self.connect(button, QtCore.SIGNAL("stateChanged(int)"), self.toggle_visibility)
        else:
            button.top_level_item = row
            self.connect(button, QtCore.SIGNAL("stateChanged(int)"), self.toggle_all_visibility)

        button = QtGui.QCheckBox("selectable")
        button.setAutoFillBackground(True)
        button.setCheckState(QtCore.Qt.Checked)
        self.checkables_tree.setItemWidget(row, Tool_box.SELECTABILITY_BUTTON_COLUMN, button)
        if not is_top_level:
            button.items = row.items
            self.connect(button, QtCore.SIGNAL("stateChanged(int)"), self.toggle_selectability)
        else:
            button.top_level_item = row
            self.connect(button, QtCore.SIGNAL("stateChanged(int)"), self.toggle_all_selectability)

    def toggle_visibility(self, status):
        button = self.sender()
        for item in button.items:
            if QtCore.Qt.Checked == status:
                item.show()
            else:
                item.hide()

    def toggle_all_visibility(self, status):
        button = self.sender()
        top_level_item = button.top_level_item
        for i in range(top_level_item.childCount()):
            child = top_level_item.child(i)
            check_box = self.checkables_tree.itemWidget(child, Tool_box.VISIBILITY_BUTTON_COLUMN)
            check_box.setCheckState(status)

    def toggle_selectability(self, status):
        button = self.sender()
        for item in button.items:
            if QtCore.Qt.Checked == status:
                item.is_selectable = True
            else:
                item.is_selectable = False

    def toggle_all_selectability(self, status):
        button = self.sender()
        top_level_item = button.top_level_item
        for i in range(top_level_item.childCount()):
            child = top_level_item.child(i)
            check_box = self.checkables_tree.itemWidget(child, Tool_box.SELECTABILITY_BUTTON_COLUMN)
            check_box.setCheckState(status)

    def make_point_or_line_button(self):
        button = QtGui.QPushButton("Snap to lines")
        button.setCheckable(True)
        button.setChecked(True)
        self.connect(button, QtCore.SIGNAL("toggled(bool)"), self.toggle_point_or_line)
        self.point_or_line_button = button

    def toggle_point_or_line(self, is_checked):
        button = self.sender()
        if is_checked:
            button.setText("Snap to lines")
        else:
            button.setText("Snap to points")
