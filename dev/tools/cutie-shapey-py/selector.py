#!/usr/bin/env python

# Copyright Singapore-MIT Alliance for Research and Technology

from PyQt4 import QtGui, QtCore
import arrow
import lane
import kerb
import road

class Selector(QtGui.QDockWidget):
    def __init__(self, parent=None):
        super(Selector, self).__init__("Selector", parent)
        self.setFeatures(self.features() & ~QtGui.QDockWidget.DockWidgetClosable)

        widget = QtGui.QWidget()
        self.setWidget(widget)

        layout = QtGui.QVBoxLayout()
        widget.setLayout(layout)
        layout.addWidget(self.make_checkables())

    def make_checkables(self):
        tree = QtGui.QTreeWidget()
        tree.setHeaderHidden(True)
        tree.setColumnCount(3)
        self.checkables_tree = tree

        self.road_group = self.add_group("Roads")
        self.road_type_group = self.add_group("Road types")
        self.arrow_marking_group = self.add_group("Arrow markings")
        self.lane_marking_group = self.add_group("Lane markings")
        self.kerb_line_group = self.add_kerb_line_group("Kerb lines")

        return tree

    def add_group(self, group_name):
        row = QtGui.QTreeWidgetItem([group_name])
        self.checkables_tree.addTopLevelItem(row)
        return row

    def add_kerb_line_group(self, group_name):
        row = QtGui.QTreeWidgetItem(["button", "color"])
        self.checkables_tree.addTopLevelItem(row)
        row.items = list()
        self.make_check_box_and_combo_box(group_name, row, "black", "line")

        return row

    def add_checkable(self, item, rec):
        if isinstance(rec, arrow.Arrow_marking):
            self.add_type(self.arrow_marking_group, item, rec, "green", "polygon")
        elif isinstance(rec, lane.Lane_marking):
            self.add_type(self.lane_marking_group, item, rec, "blue", "line")
        elif isinstance(rec, kerb.Kerb_line):
            self.kerb_line_group.items.append(item)
        elif isinstance(rec, road.Road):
            self.add_road(self.road_group, item, rec, "rec", "line")
            self.add_type(self.road_type_group, item, rec, "red", "line")

    def add_type(self, group, item, rec, initial_color, shape_type):
        for i in range(group.childCount()):
            child = group.child(i)
            widget = self.checkables_tree.itemWidget(child, 0)
            if rec.type == widget.text():
                child.items.append(item)
                break
        else:
            child = QtGui.QTreeWidgetItem(["button", "color", rec.type_desc()])
            group.addChild(child)

            child.items = list()
            child.items.append(item)

            self.make_check_box_and_combo_box(rec.type, child, initial_color, shape_type)

    def add_road(self, group, item, rec, initial_color, shape_type):
        for i in range(group.childCount()):
            child = group.child(i)
            widget = self.checkables_tree.itemWidget(child, 0)
            if rec.name == widget.text():
                child.items.append(item)
                break
        else:
            child = QtGui.QTreeWidgetItem(["button", "color", rec.name])
            group.addChild(child)

            child.items = list()
            child.items.append(item)

            self.make_check_box_and_combo_box(rec.name, child, initial_color, shape_type)

    def make_check_box_and_combo_box(self, name, row, initial_color, shape_type):
        button = QtGui.QCheckBox(name)
        button.setAutoFillBackground(True)
        button.setCheckState(QtCore.Qt.Checked)
        button.items = row.items
        self.checkables_tree.setItemWidget(row, 0, button)
        self.connect(button, QtCore.SIGNAL("stateChanged(int)"), self.toggle_visibility)

        combo = QtGui.QComboBox()
        combo.addItems(QtGui.QColor.colorNames())
        combo.shape_type = shape_type
        index = combo.findText(initial_color)
        if -1 != index:
            combo.setCurrentIndex(index)
        combo.items = row.items
        self.checkables_tree.setItemWidget(row, 1, combo)
        self.connect(combo, QtCore.SIGNAL("activated(const QString)"), self.change_color)

    def toggle_visibility(self, status):
        button = self.sender()
        for item in button.items:
            if QtCore.Qt.Checked == status:
                item.show()
            else:
                item.hide()

    def change_color(self, color_name):
        combo = self.sender()
        for item in combo.items:
            if "polygon" == combo.shape_type:
                item.setBrush(QtGui.QColor(color_name))
            else:
                item.setPen(QtGui.QColor(color_name))
