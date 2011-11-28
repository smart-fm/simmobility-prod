#!/usr/bin/env python

# Copyright Singapore-MIT Alliance for Research and Technology

from PyQt4 import QtGui, QtCore
import math

from section import Section
from crossing import Crossing
from lane import Lane
from selector import Row, Selector
# Colors are imported into the local scope, no namespace
from colors import *

class GraphicsView(QtGui.QGraphicsView):
    def __init__(self, parent=None):
        # Based on code from "Rapid GUI programming with Python and Qt" by Mark Summerfield

        super(GraphicsView, self).__init__(parent)

        self.setDragMode(QtGui.QGraphicsView.ScrollHandDrag)
        self.setRenderHint(QtGui.QPainter.Antialiasing)
        self.setRenderHint(QtGui.QPainter.TextAntialiasing)

        # For the geospatial data, the Y-axis points upwards.  QtGui.QGraphicsView has its Y-axis
        # pointing down.  Flip the view about the X-Axis so that things look right.
        matrix = QtGui.QTransform()
        matrix.rotate(180, QtCore.Qt.XAxis)
        self.setTransform(matrix, True)

        # When user presses 'C' or 'A' to rotate the view, we maintain the orientation field
        # so that we can revert to the original orientation when the user presses 'O' (letter O).
        self.orientation = 0

    def setScene(self, scene):
        rect = scene.sceneRect()
        size = self.viewport().size()
        factor1 = size.width() / rect.width()
        factor2 = size.height() / rect.height()
        factor = factor1 if factor1 < factor2 else factor2
        self.scale(factor, factor)
        super(GraphicsView, self).setScene(scene)

    def wheelEvent(self, event):
        factor = 1.41 ** (-event.delta() / 240.0)
        self.scale(factor, factor)

    def keyPressEvent(self, event):
        if event.key() == QtCore.Qt.Key_Plus:
            self.scale(1.1, 1.1)
        # too lazy to press the <Shift> to get '+'.  So let Key_Equal do the same thing.
        elif event.key() == QtCore.Qt.Key_Equal:
            self.scale(1.1, 1.1)
        elif event.key() == QtCore.Qt.Key_Minus:
            self.scale(0.91, 0.91)
        elif event.key() == QtCore.Qt.Key_Control:
            self.setDragMode(QtGui.QGraphicsView.RubberBandDrag)
        # Make it difficult to rotate the view by assigning capital 'C', 'A", and 'O' (letter O).
        elif event.key() == QtCore.Qt.Key_C and event.modifiers() == QtCore.Qt.ShiftModifier:
            # Rotate the view clockwise by 1 degree
            self.rotate(-1)
            self.orientation -= 1
        elif event.key() == QtCore.Qt.Key_A and event.modifiers() == QtCore.Qt.ShiftModifier:
            # Rotate the view anti-clockwise by 1 degree
            self.rotate(1)
            self.orientation += 1
        elif event.key() == QtCore.Qt.Key_O and event.modifiers() == QtCore.Qt.ShiftModifier:
            # Revert back to the original orientation
            self.rotate(-self.orientation)
            self.orientation = 0

class MainWindow(QtGui.QMainWindow):
    def __init__(self, network, parent=None):
        super(MainWindow, self).__init__(parent)

        self.view = GraphicsView()
        self.scene = QtGui.QGraphicsScene(self)
        #self.scene.setSceneRect(network.left, network.bottom,
        #                        network.right - network.left, network.top - network.bottom)

        self.setCentralWidget(self.view)
        self.selector = Selector(self)
        self.addDockWidget(QtCore.Qt.LeftDockWidgetArea, self.selector)

        self.setWindowTitle("Snake eye view of the streets of Singapore")

        self.add_network(network)
        self.view.setScene(self.scene)

    def add_network(self, network):
        self.polyline_groups = list()
        self.add_roads(network.roads)

    def add_roads(self, roads):
        for name, sections in roads.iteritems():
            row = Row(name.title())
            for section in sections:
                row += self.add_section(section)
            self.selector.add_road(row)

    def add_section(self, section):
        row = Row("section = %s" % section.id)
        row += self.add_section_node(section.from_node, "from-node")
        row += self.add_section_node(section.to_node, "to-node")

        row += Row("length=%d, num-lanes=%d" % (section.length, section.num_lanes))
        row += Row("speed=%d, capacity=%d" % (section.speed, section.capacity))

        row += self.add_section_polyline(section.polyline)
        for lane_id, crossings in section.crossings.iteritems():
            for lane_type, crossing in crossings.iteritems():
                row += self.add_crossing(lane_id, lane_type, crossing)
        for lane_id, lanes in section.lanes.iteritems():
            for lane_type, lane in lanes.iteritems():
                row += self.add_lane(lane_id, lane_type, lane)

        return row

    def add_section_node(self, node, prefix):
        item = self.create_plus_sign(node, QtCore.Qt.blue, 50)
        item.hide()
        row = Row("%s = (%d, %d)" % (prefix, node.x, node.y), item)
        return row

    def add_section_polyline(self, polyline):
        if len(polyline) > 2:
            self.arrange_by_x(polyline)

        row = Row("section polyline")
        for point in polyline:
            row += self.add_point(point)

        group = QtGui.QGraphicsItemGroup()
        for i in range(len(polyline) - 1):
            p1 = polyline[i]
            p2 = polyline[i + 1]
            line = self.scene.addLine(p1.x, p1.y, p2.x, p2.y, blue)
            group.addToGroup(line)
        self.scene.addItem(group)
        row.data = group

        self.selector.polylines.append(group)
        if self.selector.are_polylines_hidden():
            group.hide()

        return row

    def add_point(self, point, hide=True):
        item = self.create_plus_sign(point, red, 50)
        if hide:
            item.hide()
        row = Row("(%d, %d)" % (point.x, point.y), item)
        return row

    def add_crossing(self, lane_id, lane_type, points):
        row = Row("lane-id=%s, lane-type=%s \"(crossing)\"" % (lane_id, lane_type))

        for point in points:
            row += self.add_point(point)

        # Uncomment the next 2 lines to make "Victoria-Street/34450/4550" looks nice. 
        #if len(points) > 2:
        #    self.arrange_by_x(points)

        group = QtGui.QGraphicsItemGroup()
        for i in range(len(points) - 1):
            p1 = points[i]
            p2 = points[i + 1]
            line = self.scene.addLine(p1.x, p1.y, p2.x, p2.y, blue)
            group.addToGroup(line)
        self.scene.addItem(group)
        row.data = group

        self.selector.crossings.append(group)
        if self.selector.are_crossings_hidden():
            group.hide()

        return row

    def arrange_by_x(self, points):
        """sort the points by their x co-ordinate, in ascending order"""
        points.sort(key = lambda point : point.x)

    def arrange_by_y(self, points):
        """sort the points by their y co-ordinate, in ascending order"""
        points.sort(key = lambda point : point.y)

    def add_lane(self, lane_id, lane_type, points):
        row = Row("lane-id=%s, lane-type=%s \"(lane)\"" % (lane_id, lane_type))

        for point in points:
            row += self.add_point(point, False)

        if lane_type == 'I':
            row.data = self.add_kerb_line(points)

        return row

    def add_kerb_line(self, points):
        if len(points) > 2:
            self.arrange_by_x(points)

        group = QtGui.QGraphicsItemGroup()
        for i in range(len(points) - 1):
            p1 = points[i]
            p2 = points[i + 1]
            line = self.scene.addLine(p1.x, p1.y, p2.x, p2.y, magenta)
            group.addToGroup(line)
        self.scene.addItem(group)

        self.selector.kerb_lines.append(group)
        if self.selector.are_kerb_lines_hidden():
            group.hide()

        return group

    def create_plus_sign(self, pt, color=red, width=100):
        horizontal = self.scene.addLine(pt.x - width, pt.y, pt.x + width, pt.y, color)
        vertical = self.scene.addLine(pt.x, pt.y - width, pt.x, pt.y + width, color)
        group = QtGui.QGraphicsItemGroup()
        group.addToGroup(horizontal)
        group.addToGroup(vertical)
        self.scene.addItem(group)
        return group
