#!/usr/bin/env python

# Copyright (2012) Singapore-MIT Alliance for Research and Technology

from PyQt4 import QtGui, QtCore

from ui_edit_form import Ui_edit_form
from ui_multiple_edit_form import Ui_multiple_edit_form
from digital_map import Road_section, Lane_marking, Kerb_line
from network import Lane_edge
from chain import Ball, Chain
from point import Point, simplify_polyline

class Edit_form(QtGui.QDialog, Ui_edit_form):
    def __init__(self, road_item, main_window):
        super(Edit_form, self).__init__(parent=main_window)
        self.setupUi(self)
        self.setModal(False)

        self.road_item = road_item
        if isinstance(road_item, Road_section):
            self.prepare_to_edit_road_section(road_item, main_window.digital_map)
        elif isinstance(road_item, Lane_marking):
            self.prepare_to_edit_lane_marking(road_item, main_window.digital_map)
        elif isinstance(road_item, Kerb_line):
            self.prepare_to_edit_kerb_line(road_item)
        elif isinstance(road_item, Lane_edge):
            self.prepare_to_edit_lane_edge(road_item, main_window.road_network)
        self.resize(300, 500)
        self.editable_item = Editable_polyline(road_item.visual_item, self.polyline_point_was_moved)

    def prepare_to_edit_road_section(self, road_section, digital_map):
        self.setWindowTitle("pqrne: edit road-section attributes")

        for road_name in sorted(digital_map.roads.keys()):
            self.name_combo_box.addItem(road_name)
        for road_type in sorted(digital_map.road_types.keys()):
            type_description = digital_map.road_types[road_type]
            self.type_combo_box.addItem(type_description, road_type)

        index = self.name_combo_box.findText(road_section.road_name)
        self.name_combo_box.setCurrentIndex(index)
        index = self.type_combo_box.findData(road_section.road_type)
        self.type_combo_box.setCurrentIndex(index)
        self.populate_polyline_tree_widget(road_section.polyline)

    def prepare_to_edit_lane_marking(self, lane_marking, digital_map):
        self.setWindowTitle("pqrne: edit lane-marking attributes")

        self.name_label.setVisible(False)
        self.name_combo_box.setVisible(False)
        self.type_label.setText("Marking type:")
        for marking_type in sorted(digital_map.lane_marking_types.keys()):
            type_description = digital_map.lane_marking_types[marking_type]
            self.type_combo_box.addItem(type_description, marking_type)

        index = self.type_combo_box.findData(lane_marking.marking_type)
        self.type_combo_box.setCurrentIndex(index)
        self.populate_polyline_tree_widget(lane_marking.polyline)

    def prepare_to_edit_kerb_line(self, kerb_line):
        self.setWindowTitle("pqrne: edit kerb-line attributes")

        self.name_label.setVisible(False)
        self.name_combo_box.setVisible(False)
        self.type_label.setVisible(False)
        self.type_combo_box.setVisible(False)

        self.populate_polyline_tree_widget(kerb_line.polyline)

    def prepare_to_edit_lane_edge(self, lane_edge, road_network):
        self.setWindowTitle("pqrne: edit lane-edge attributes")

        self.name_label.setVisible(False)
        self.name_combo_box.setVisible(False)
        self.type_label.setText("Marking type:")
        for marking_type in sorted(road_network.lane_edge_types.keys()):
            type_description = road_network.lane_edge_types[marking_type]
            self.type_combo_box.addItem(type_description, marking_type)

        index = self.type_combo_box.findData(lane_edge.marking_type)
        self.type_combo_box.setCurrentIndex(index)
        self.populate_polyline_tree_widget(lane_edge.polyline)

    def populate_polyline_tree_widget(self, polyline):
        for i, point in enumerate(polyline):
            columns = "%d %.2f %.2f %.2f" % (i, point.x, point.y, point.z)
            row = QtGui.QTreeWidgetItem(columns.split())
            row.setFlags(row.flags() | QtCore.Qt.ItemIsEditable)
            self.polyline_tree_widget.addTopLevelItem(row)
        header = self.polyline_tree_widget.header()
        header.resizeSections(QtGui.QHeaderView.ResizeToContents)

    def polyline_point_was_moved(self, index, x, y):
        row = self.polyline_tree_widget.topLevelItem(index)
        row.setText(1, "%.2f" % x)
        row.setText(2, "%.2f" % y)
        self.set_dirty()

    @QtCore.pyqtSignature("int")
    def on_name_combo_box_activated(self, index):
        new_choice = self.name_combo_box.itemText(index)
        if new_choice != self.road_section.road_name:
            self.set_dirty()

    @QtCore.pyqtSignature("int")
    def on_type_combo_box_activated(self, index):
        new_choice = self.type_combo_box.itemData(index).toString()
        if isinstance(self.road_item, Road_section) and new_choice != self.road_item.road_type:
            self.set_dirty()
        elif isinstance(self.road_item, Lane_marking) and new_choice != self.road_item.marking_type:
            self.set_dirty()
        elif isinstance(self.road_item, Lane_edge) and new_choice != self.road_item.marking_type:
            self.set_dirty()

    @QtCore.pyqtSignature("QTreeWidgetItem*, int")
    def on_polyline_tree_widget_itemChanged(self, row, column):
        self.set_dirty()

    def set_dirty(self):
        self.warning_label.setText("You have modified at least one of the attributes.  "
            "Click the <Cancel> button to reject the changes.  If you click the <Ok> button, "
            "the changes will be saved to the shape-file database.")

    @QtCore.pyqtSignature("")
    def on_button_box_accepted(self):
        if isinstance(self.road_item, Road_section):
            road_name = str(self.name_combo_box.currentText())
            if self.road_item.road_name != road_name:
                self.road_item.road_name = road_name
            index = self.type_combo_box.currentIndex()
            road_type = str(self.type_combo_box.itemData(index).toString())
            if self.road_item.road_type != road_type:
                self.road_item.road_type = road_type
            polyline = self.check_polyline()
            if polyline:
                self.road_item.polyline = polyline
                self.change_graphics_item(polyline)
        elif isinstance(self.road_item, Lane_marking):
            index = self.type_combo_box.currentIndex()
            marking_type = str(self.type_combo_box.itemData(index).toString())
            if self.road_item.marking_type != marking_type:
                self.road_item.marking_type = marking_type
            polyline = self.check_polyline()
            if polyline:
                self.road_item.polyline = polyline
                self.change_graphics_item(polyline)
        elif isinstance(self.road_item, Kerb_line):
            polyline = self.check_polyline()
            if polyline:
                self.road_item.polyline = polyline
                self.change_graphics_item(polyline)
        elif isinstance(self.road_item, Lane_edge):
            index = self.type_combo_box.currentIndex()
            marking_type = str(self.type_combo_box.itemData(index).toString())
            if self.road_item.marking_type != marking_type:
                self.road_item.marking_type = marking_type
            polyline = self.check_polyline()
            if polyline:
                self.road_item.polyline = polyline
                self.change_graphics_item(polyline)
        self.finish_editing()

    def check_polyline(self):
        polyline = self.editable_item.polyline()
        for i in range(self.polyline_tree_widget.topLevelItemCount()):
            row = self.polyline_tree_widget.topLevelItem(i)
            polyline[i].z = float(row.text(3))
        polyline = simplify_polyline(polyline)
        if len(self.road_item.polyline) != len(polyline):
            return polyline
        for i in range(len(polyline)):
            if not self.road_item.polyline[i].is_almost_equal(polyline[i]):
                return polyline
        return None

    def change_graphics_item(self, polyline):
        point = polyline[0]
        path = QtGui.QPainterPath(QtCore.QPointF(point.x, point.y))
        for point in polyline[1:]:
            path.lineTo(point.x, point.y)
        self.road_item.visual_item.setPath(path)

    @QtCore.pyqtSignature("")
    def on_button_box_rejected(self):
        self.finish_editing()

    def finish_editing(self):
        self.editable_item = None
        self.emit(QtCore.SIGNAL("finish_editing"))

class Cached_item:
    def __init__(self, index, road_item):
        self.index = index
        self.road_item = road_item
        self.name = None
        self.type = None
        self.polyline = None

    def is_dirty(self):
        return (self.name != None or self.type != None or self.polyline != None)

    def cache_all_changes(self, name_combo_box, type_combo_box, polyline_tree_widget,
                          editable_item):
        if isinstance(self.road_item, Road_section):
            road_name = str(name_combo_box.currentText())
            if self.road_item.road_name != road_name:
                self.name = road_name
            else:
                self.name = None
            index = type_combo_box.currentIndex()
            road_type = str(type_combo_box.itemData(index).toString())
            if self.road_item.road_type != road_type:
                self.type = road_type
            else:
                self.type = None
            polyline = self.check_polyline(editable_item, polyline_tree_widget)
            if polyline:
                self.polyline = polyline
            else:
                self.polyline = None
        elif isinstance(self.road_item, Lane_marking):
            index = type_combo_box.currentIndex()
            road_type = str(type_combo_box.itemData(index).toString())
            if self.road_item.road_type != road_type:
                self.type = road_type
            else:
                self.type = None
            polyline = self.check_polyline(editable_item, polyline_tree_widget)
            if polyline:
                self.polyline = polyline
            else:
                self.polyline = None
        elif isinstance(self.road_item, Kerb_line):
            polyline = self.check_polyline(editable_item, polyline_tree_widget)
            if polyline:
                self.polyline = polyline
            else:
                self.polyline = None
        elif isinstance(self.road_item, Lane_edge):
            index = type_combo_box.currentIndex()
            road_type = str(type_combo_box.itemData(index).toString())
            if self.road_item.road_type != road_type:
                self.type = road_type
            else:
                self.type = None
            polyline = self.check_polyline(editable_item, polyline_tree_widget)
            if polyline:
                self.polyline = polyline
            else:
                self.polyline = None

    def check_polyline(self, editable_item, polyline_tree_widget):
        polyline = editable_item.polyline()
        for i in range(polyline_tree_widget.topLevelItemCount()):
            row = polyline_tree_widget.topLevelItem(i)
            polyline[i].z = float(row.text(3))
        if len(self.road_item.polyline) != len(polyline):
            return polyline
        for i in range(len(polyline)):
            if not self.road_item.polyline[i].is_almost_equal(polyline[i]):
                return polyline
        return None

class Multiple_edit_form(QtGui.QDialog, Ui_multiple_edit_form):
    def __init__(self, road_items, main_window):
        super(Multiple_edit_form, self).__init__(parent=main_window)
        self.setupUi(self)
        self.setModal(False)
        self.main_window = main_window

        self.cached_items = list()
        self.current_road_item = None

        for i, road_item in enumerate(road_items):
            self.cached_items.append(Cached_item(i, road_item))

            if isinstance(road_item, Road_section):
                column_2 = "road type='%s' name='%s'" % (road_item.road_type, road_item.road_name)
            elif isinstance(road_item, Lane_marking):
                column_2 = "lane-marking type='%s'" % road_item.marking_type
            elif isinstance(road_item, Kerb_line):
                column_2 = "kerb-line"
            elif isinstance(road_item, Lane_edge):
                column_2 = "lane-edge type='%s'" % road_item.marking_type
            row = QtGui.QTreeWidgetItem(["%d" % i, "Delete this", column_2])
            self.edit_items_tree_widget.addTopLevelItem(row)
            header = self.edit_items_tree_widget.header()
            header.setResizeMode(0, QtGui.QHeaderView.ResizeToContents)
            header.setResizeMode(1, QtGui.QHeaderView.ResizeToContents)
            check_box = QtGui.QCheckBox("Delete this")
            check_box.setAutoFillBackground(True)
            check_box.setCheckState(QtCore.Qt.Unchecked)
            self.edit_items_tree_widget.setItemWidget(row, 1, check_box)
        self.edit_items_tree_widget.setCurrentItem(self.edit_items_tree_widget.topLevelItem(0))

    @QtCore.pyqtSignature("QTreeWidgetItem*, QTreeWidgetItem*")
    def on_edit_items_tree_widget_currentItemChanged(self, current, previous):
        if self.current_road_item:
            self.current_road_item.cache_all_changes(self.name_combo_box, self.type_combo_box,
                                                     self.polyline_tree_widget, self.editable_item)
            column_0_text = "%d" % self.current_road_item.index
            if self.current_road_item.is_dirty():
                column_0_text = column_0_text + " *"
            previous.setText(0, column_0_text)

        index = self.edit_items_tree_widget.indexOfTopLevelItem(current)
        self.current_road_item = self.cached_items[index]

        road_item = self.current_road_item.road_item
        if isinstance(road_item, Road_section):
            self.prepare_to_edit_road_section(road_item, self.main_window.digital_map)
        elif isinstance(road_item, Lane_marking):
            self.prepare_to_edit_lane_marking(road_item, self.main_window.digital_map)
        elif isinstance(road_item, Kerb_line):
            self.prepare_to_edit_kerb_line(road_item)
        self.editable_item = Editable_polyline(road_item.visual_item,
                                               self.polyline_point_was_moved,
                                               self.current_road_item.polyline)

    def prepare_to_edit_road_section(self, road_section, digital_map):
        self.name_combo_box.clear()
        for road_name in sorted(digital_map.roads.keys()):
            self.name_combo_box.addItem(road_name)

        self.type_combo_box.clear()
        for road_type in sorted(digital_map.road_types.keys()):
            type_description = digital_map.road_types[road_type]
            self.type_combo_box.addItem(type_description, road_type)

        self.name_label.setVisible(True)
        self.name_combo_box.setVisible(True)
        if self.current_road_item.name:
            index = self.name_combo_box.findText(self.current_road_item.name)
        else:
            index = self.name_combo_box.findText(road_section.road_name)
        self.name_combo_box.setCurrentIndex(index)

        self.name_label.setText("Road type:")
        self.type_label.setVisible(True)
        self.type_combo_box.setVisible(True)
        if self.current_road_item.type:
            index = self.type_combo_box.findData(self.current_road_item.type)
        else:
            index = self.type_combo_box.findData(road_section.road_type)
        self.type_combo_box.setCurrentIndex(index)

        if self.current_road_item.polyline:
            self.populate_polyline_tree_widget(self.current_road_item.polyline)
        else:
            self.populate_polyline_tree_widget(road_section.polyline)

    def prepare_to_edit_lane_marking(self, lane_marking, digital_map):
        self.name_label.setVisible(False)
        self.name_combo_box.setVisible(False)

        self.type_combo_box.clear()
        for marking_type in sorted(digital_map.lane_marking_types.keys()):
            type_description = digital_map.lane_marking_types[marking_type]
            self.type_combo_box.addItem(type_description, marking_type)

        self.type_label.setText("Marking type:")
        self.type_label.setVisible(True)
        self.type_combo_box.setVisible(True)
        if self.current_road_item.type:
            index = self.type_combo_box.findData(self.current_road_item.type)
        else:
            index = self.type_combo_box.findData(lane_marking.marking_type)
        self.type_combo_box.setCurrentIndex(index)

        if self.current_road_item.polyline:
            self.populate_polyline_tree_widget(self.current_road_item.polyline)
        else:
            self.populate_polyline_tree_widget(lane_marking.polyline)

    def prepare_to_edit_kerb_line(self, kerb_line):
        self.name_label.setVisible(False)
        self.name_combo_box.setVisible(False)
        self.type_label.setVisible(False)
        self.type_combo_box.setVisible(False)

        if self.current_road_item.polyline:
            self.populate_polyline_tree_widget(self.current_road_item.polyline)
        else:
            self.populate_polyline_tree_widget(kerb_line.polyline)

    def populate_polyline_tree_widget(self, polyline):
        self.polyline_tree_widget.clear()
        for i, point in enumerate(polyline):
            columns = "%d %.2f %.2f %.2f" % (i, point.x, point.y, point.z)
            row = QtGui.QTreeWidgetItem(columns.split())
            row.setFlags(row.flags() | QtCore.Qt.ItemIsEditable)
            self.polyline_tree_widget.addTopLevelItem(row)
        header = self.polyline_tree_widget.header()
        header.resizeSections(QtGui.QHeaderView.ResizeToContents)

    def polyline_point_was_moved(self, index, x, y):
        row = self.polyline_tree_widget.topLevelItem(index)
        row.setText(1, "%.2f" % x)
        row.setText(2, "%.2f" % y)

    @QtCore.pyqtSignature("int")
    def on_name_combo_box_activated(self, index):
        pass

    @QtCore.pyqtSignature("int")
    def on_type_combo_box_activated(self, index):
        pass

    @QtCore.pyqtSignature("")
    def on_button_box_rejected(self):
        self.finish_editing()

    def finish_editing(self):
        self.editable_item = None
        self.emit(QtCore.SIGNAL("finish_editing"))

class Editable_polyline:
    def __init__(self, graphics_item, slot, use_this_polyline=None):
        self.graphics_item = graphics_item
        self.graphics_item.setVisible(False)

        road_item = graphics_item.road_item
        graphics_scene = graphics_item.scene()

        self.balls = list()
        if use_this_polyline:
            polyline = use_this_polyline
        else:
            polyline = road_item.polyline
        for i, point in enumerate(polyline):
            ball = Ball(point.x, point.y, Observer(slot, i))
            ball.is_selectable = False
            graphics_scene.addItem(ball)
            self.balls.append(ball)

        self.chains = list()
        for i in range(len(self.balls) - 1):
            ball1 = self.balls[i]
            ball2 = self.balls[i+1]
            chain = Chain(ball1, ball2)
            chain.is_selectable = False
            graphics_scene.addItem(chain)
            self.chains.append(chain)

    def polyline(self):
        result = list()
        for ball in self.balls:
            position = ball.pos()
            result.append(Point(position.x(), position.y()))
        return result

    def __del__(self):
        graphics_scene = self.graphics_item.scene()
        for ball in self.balls:
            graphics_scene.removeItem(ball)
        for chain in self.chains:
            graphics_scene.removeItem(chain)
        self.graphics_item.setVisible(True)

class Observer:
    def __init__(self, slot, index):
        self.slot = slot
        self.index = index

    def notify(self, x, y):
        self.slot(self.index, x, y)
