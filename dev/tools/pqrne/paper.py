#!/usr/bin/env python

# Copyright (2012) Singapore-MIT Alliance for Research and Technology

import math
from PyQt4 import QtGui, QtCore

class Tracing_paper(QtGui.QGraphicsScene):
    def __init__(self, parent=None):
        super(Tracing_paper, self).__init__(parent)
        self.graphics_items = dict()

    def mark_out_polyline(self, road_item):
        point = road_item.polyline[0]
        path = QtGui.QPainterPath(QtCore.QPointF(point.x, point.y))
        for point in road_item.polyline[1:]:
            path.lineTo(point.x, point.y)
        graphics_item = QtGui.QGraphicsPathItem(path)
        graphics_item.road_item = road_item
        self.addItem(graphics_item)
        self.graphics_items[road_item] = graphics_item
        return graphics_item

    def mark_out_position(self, road_item):
        w = 0.001
        h = 0.001
        graphics_item = QtGui.QGraphicsRectItem(-w/2.0, -h/2.0, w, h)
        graphics_item.setPos(road_item.position.x, road_item.position.y)
        graphics_item.road_item = road_item
        self.addItem(graphics_item)
        self.graphics_items[road_item] = graphics_item
        return graphics_item

    def hide(self, road_item):
        graphics_item = self.graphics_items[road_item]
        graphics_item.setVisible(False)

    def show(self, road_item):
        graphics_item = self.graphics_items[road_item]
        graphics_item.setVisible(True)

    def graphics_items_around_point(self, point, w, h):
        return self.items(point.x - w / 2.0, point.y - h / 2.0, w, h)

    def road_items_around_point(self, point, w, h):
        return [item.road_item for item in self.graphics_items_around_point(point, w, h)]

    def graphics_items_in_bounding_box(self, point1, point2):
        x1 = point1.x
        x2 = point2.x
        y1 = point1.y
        y2 = point2.y
        if x1 > x2:
            x = x2
            w = x1 - x2
        else:
            x = x1
            w = x2 - x1
        if y1 > y2:
            y = y2
            h = y1 - y2
        else:
            y = y1
            h = y2 - y1
        return self.items(x, y, w, h)

    def road_items_in_bounding_box(self, point1, point2):
        return [item.road_item for item in self.graphics_items_in_bounding_box(point1, point2)]

    def graphics_items_intersecting_polyline(self, polyline):
        path = QtGui.QPainterPath(QtCore.QPointF(polyline[0].x, polyline[0].y))
        for point in polyline[1:]:
            path.lineTo(point.x, point.y)
        return self.items(path)

    def road_items_intersecting_polyline(self, polyline):
        return [item.road_item for item in self.graphics_items_intersecting_polyline(polyline)]

    def graphics_items_left_or_right_of_line(self, point1, point2, width):
        dx = point2.x - point1.x
        dy = point2.y - point1.y
        mag = math.hypot(dx, dy)
        dx = dx * width / mag
        dy = dy * width / mag

        graphics_items = set()

        p1 = Point(point1.x - dy, point1.y + dx)
        p2 = Point(point2.x - dy, point2.y + dx)
        polyline = (p1, p2)
        for item in self.graphics_items_intersecting_polyline(polyline):
            graphics_items.add(item)

        p1 = Point(point1.x + dy, point1.y - dx)
        p2 = Point(point2.x + dy, point2.y - dx)
        polyline = (p1, p2)
        for item in self.graphics_items_intersecting_polyline(polyline):
            graphics_items.add(item)

        return graphics_items

    def road_items_left_or_right_of_line(self, point1, point2, width):
        return [item.road_item for item in self.graphics_items_left_or_right_of_line(point1, point2,
                                                                                     width)]

    def graphics_items_around_line(self, point1, point2, extension, width):
        dx = point2.x - point1.x
        dy = point2.y - point1.y
        mag = math.hypot(dx, dy)
        x = dx * extension / mag
        y = dy * extension / mag
        dx = dx * width / mag
        dy = dy * width / mag

        path = QtGui.QPainterPath(QtCore.QPointF(point1.x - x - dy, point1.y - y + dx))
        path.lineTo(point1.x - x + dy, point1.y - y - dx)
        path.lineTo(point2.x + x + dy, point2.y + y - dx)
        path.lineTo(point2.x + x - dy, point2.y + y + dx)
        path.closeSubpath()

        return self.items(path)

    def road_items_around_line(self, point1, point2, extension, width):
        return [item.road_item for item in self.graphics_items_around_line(point1, point2,
                                                                           extension, width)]

# vim:columns=100:smartindent:shiftwidth=4:expandtab:softtabstop=4:
