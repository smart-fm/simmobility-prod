#!/usr/bin/python

# Copyright Singapore-MIT Alliance for Research and Technology

import math
from PyQt4 import QtGui, QtCore

from scrapings import Scrapings
import edge
from dialog import Dialogs
from chain import Ball
from simmob.point import Point

class Selected_lane_edge:
    def __init__(self, graphics_item):
        self.lane_edge = graphics_item.lane_edge
        self.graphics_items = list()
        graphics_item.get_siblings(self.graphics_items)
        for item in self.graphics_items:
            item.set_color(QtCore.Qt.red)

    def clear(self, lane_edge):
        if isinstance(lane_edge, edge.Lane_edge):
            color = QtCore.Qt.blue
        elif isinstance(lane_edge, edge.Center_line):
            color = QtGui.QColor("gray")
        for item in self.graphics_items:
            item.set_color(color)

class Graphics_view(QtGui.QGraphicsView):
    def __init__(self, main_window, road_network, parent=None):
        super(Graphics_view, self).__init__(parent)
        self.scale(0.00925, 0.00925)
        self.status_bar = main_window.statusBar()
        self.tool_box = main_window.tool_box
        self.road_network = road_network

        self.setDragMode(QtGui.QGraphicsView.ScrollHandDrag)
        #self.setCursor(QtGui.QCursor(QtCore.Qt.ArrowCursor))
        self.setRenderHint(QtGui.QPainter.Antialiasing)
        self.setRenderHint(QtGui.QPainter.TextAntialiasing)

        # Flip the view about the X-axis so that the view is in Cartesian co-ordinate system
        # where the Y-axis points upwards.
        matrix = QtGui.QTransform()
        matrix.rotate(180, QtCore.Qt.XAxis)
        self.setTransform(matrix, True)

        # When user presses 'C' or 'A' to rotate the view, we maintain the orientation field
        # so that we can revert to the original orientation when the user presses 'O' (letter O).
        self.orientation = 0

        self.setMouseTracking(True)
        self.scraping = None
        self.snap_to_lines = True
        self.selected_lane_edges = list()
        self.balls_are_hidden = False

    def wheelEvent(self, event):
        factor = 1.41 ** (-event.delta() / 240.0)
        self.scale(factor, factor)

    def resizeEvent(self, event):
        return

        rect = self.scene().sceneRect()
        size = event.size()
        x_factor = size.width() / rect.width()
        y_factor = size.height() / rect.height()
        factor = x_factor if x_factor > y_factor else y_factor
        self.scale(factor, factor)

    def keyPressEvent(self, event):
        if event.key() == QtCore.Qt.Key_Plus:
            self.scale(1.1, 1.1)
        # too lazy to press the <Shift> to get '+'.  So let Key_Equal do the same thing.
        elif event.key() == QtCore.Qt.Key_Equal:
            self.scale(1.1, 1.1)
        elif event.key() == QtCore.Qt.Key_Minus:
            self.scale(0.91, 0.91)
        # Make it difficult to rotate the view by assigning capital 'C', 'A", and 'O' (letter O)
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
        elif event.key() == QtCore.Qt.Key_E:
            # Edit the data of the selected lane edges or center lines, such as their
            # section-id and types.
            while len(self.selected_lane_edges):
                selected_lane_edge = self.selected_lane_edges.pop()
                lane_edge = selected_lane_edge.lane_edge
                if isinstance(lane_edge, edge.Lane_edge):
                    lane_edge_dialog = Dialogs.lane_edge_dialog
                    lane_edge_dialog.set_lane_edge(lane_edge)
                    if lane_edge_dialog.exec_():
                        lane_edge_dialog.update(lane_edge)
                    selected_lane_edge.clear(lane_edge)
                elif isinstance(lane_edge, edge.Center_line):
                    center_line_dialog = Dialogs.center_line_dialog
                    center_line_dialog.set_center_line(lane_edge)
                    if center_line_dialog.exec_():
                        center_line_dialog.update(lane_edge)
                    selected_lane_edge.clear(lane_edge)
        elif event.key() == QtCore.Qt.Key_D:
            # Delete the selected lane edges or center lines from the graphics view as well as
            # the road network.
            while len(self.selected_lane_edges):
                selected_lane_edge = self.selected_lane_edges.pop()
                for item in selected_lane_edge.graphics_items:
                    self.scene().removeItem(item)
                lane_edge = selected_lane_edge.lane_edge
                if isinstance(lane_edge, edge.Lane_edge):
                    self.road_network.lane_edges.remove(lane_edge)
                elif isinstance(lane_edge, edge.Center_line):
                    self.road_network.lane_center_lines.remove(lane_edge)
        elif event.key() == QtCore.Qt.Key_G:
            # Generate the lane's center line from the lane's 2 edges.
            if len(self.selected_lane_edges) == 2:
                selected_lane_edge = self.selected_lane_edges.pop()
                edge1 = selected_lane_edge.lane_edge
                selected_lane_edge.clear(selected_lane_edge.lane_edge)
                selected_lane_edge = self.selected_lane_edges.pop()
                edge2 = selected_lane_edge.lane_edge
                selected_lane_edge.clear(selected_lane_edge.lane_edge)
                polyline = generate_center_line(edge1, edge2);
                items = list()
                self.scene().draw_polyline(polyline, None, "center-line", items, QtCore.Qt.red)

                center_line_dialog = Dialogs.center_line_dialog
                center_line = edge.Center_line(polyline)
                center_line_dialog.set_center_line(center_line)
                if center_line_dialog.exec_():
                    center_line_dialog.update(center_line)
                    self.road_network.add_center_line(center_line)
                    self.scene().draw_center_line(center_line, self.tool_box)
                for item in items:
                    self.scene().removeItem(item)
        elif event.key() == QtCore.Qt.Key_H:
            Ball.toggle_ball_visibility()
        elif event.key() == QtCore.Qt.Key_S:
            print "saving road network"
            self.road_network.save()
        elif event.key() == QtCore.Qt.Key_N:
            self.scene().toggle_node_visibility()

    def filter_out_lower_items(self, graphics_items):
        for item in graphics_items:
            if self.is_selectable(item) and item.zValue() > 0:
                break
        else:
            result = list()
            for item in graphics_items:
                if self.is_selectable(item):
                    result.append(item)
            return result

        result = list()
        for item in graphics_items:
            if self.is_selectable(item) and item.zValue() > 0:
                result.append(item)
        return result

    def mouseMoveEvent(self, event):
        if event.buttons() == QtCore.Qt.NoButton:
            point = self.mapToScene(event.pos())
            message = "(%d, %d)" % (point.x(), point.y())
            id_list = list()
            for graphics_item in self.filter_out_lower_items(self.items(event.x(), event.y(), 5, 5)):
                message += " | %s" % graphics_item.info
            self.status_bar.showMessage(message)

        if self.scraping:
            point = self.mapToScene(event.pos())
            self.scraping.adjust(point.x(), point.y())

        event.ignore()
        super(Graphics_view, self).mouseMoveEvent(event)

    def mousePressEvent(self, event):
        if event.button() & QtCore.Qt.RightButton:
            if event.modifiers() == (QtCore.Qt.ControlModifier | QtCore.Qt.AltModifier):
                if len(self.selected_lane_edges) == 0:
                    point = self.mapToScene(event.pos())
                    object = QtCore.QObject()
                    object.road_item = Point(point.x(), point.y())
                    object.is_a_line = False
                    self.add_item_to_scraping(object)
            else:
                graphics_item = None
                for item in self.filter_out_lower_items(self.items(event.x(), event.y(), 5, 5)):
                    graphics_item = item
                    break
                if graphics_item:
                    if len(self.selected_lane_edges):
                        if graphics_item.zValue() > 0:
                            if event.modifiers() != QtCore.Qt.ShiftModifier:
                                while len(self.selected_lane_edges):
                                    selected_lane_edge = self.selected_lane_edges.pop()
                                    selected_lane_edge.clear(selected_lane_edge.lane_edge)
                            for selected_lane_edge in self.selected_lane_edges:
                                if selected_lane_edge.lane_edge == graphics_item.lane_edge:
                                    break
                            else:
                                self.selected_lane_edges.append(Selected_lane_edge(graphics_item))
                    elif graphics_item.zValue() == 0:
                        self.add_item_to_scraping(graphics_item)
                    else:
                        self.selected_lane_edges.append(Selected_lane_edge(graphics_item))
        super(Graphics_view, self).mousePressEvent(event)

    def mouseDoubleClickEvent(self, event):
        if event.button() & QtCore.Qt.RightButton:
            self.create_edge()
        super(Graphics_view, self).mouseDoubleClickEvent(event)

    def is_selectable(self, graphics_item):
        if not graphics_item.is_selectable:
            return False
        if self.snap_to_lines and graphics_item.is_a_line:
            return True
        if not self.snap_to_lines and not graphics_item.is_a_line:
            return True
        return False

    def toggle_point_or_line(self, is_checked):
        if is_checked:
            self.snap_to_lines = True
        else:
            self.snap_to_lines = False

    def add_item_to_scraping(self, graphics_item):
        if not self.scraping:
            self.scraping = Scrapings(self.scene())
        if not graphics_item.is_a_line:
            self.scraping.add_point(graphics_item.road_item)
        else:
            self.scraping.add_polyline(graphics_item.road_item.polyline)

    def create_edge(self):
        if not self.scraping:
            return

        lane_edge = self.scraping.finish()
        if lane_edge:
            self.scene().draw_lane_edge(lane_edge, self.tool_box)
            self.road_network.add_lane_edge(lane_edge)
        self.scraping = None

def generate_center_line(lane_edge1, lane_edge2):
    center_line = list()
    for point in lane_edge1.polyline:
        center_line.append(mid_way(point, lane_edge2.polyline))
    return center_line

def mid_way(point, polyline):
    class Vector:
        def __init__(self, point1, point2):
            self.x = point1.x - point2.x
            self.y = point1.y - point2.y

        def magnitude(self):
            return math.hypot(self.x, self.y)

        def scale(self, s):
            self.x *= s
            self.y *= s

        def dot_product(self, v2):
            return (self.x * v2.x) + (self.y * v2.y)

    p1, p2 = closest_line_segment(point, polyline)
    v1 = Vector(p2, p1)
    v2 = Vector(point, p1)
    dot_product = v1.dot_product(v2)
    mag = v1.magnitude()
    v1.scale(dot_product / (mag * mag))

    p = Point(p1.x, p1.y)
    p.x += v1.x
    p.y += v1.y
    v = Vector(point, p)
    v.scale(0.5)
    p.x += v.x
    p.y += v.y
    return p

def closest_line_segment(point, polyline):
    class Point:
        def __init__(self, point, reference_point):
            self.point = point
            self.distance = abs(point.x - reference_point.x) + abs(point.y - reference_point.y)

    points = list()
    p1 = polyline[0]
    points.append(Point(p1, point))
    for p in polyline[1:]:
        if abs(p.x - p1.x) > 50 and abs(p.y - p1.y) > 50:
            points.append(Point(p, point))
            p1 = p
    points.sort(key = lambda p: p.distance)
    return points[0].point, points[1].point
