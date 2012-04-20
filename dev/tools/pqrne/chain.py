#!/usr/bin/env python

# Copyright (2012) Singapore-MIT Alliance for Research and Technology

from PyQt4 import QtGui, QtCore

from point import Point, nearest_point, intersection_point, is_between

# Based on the Qt4 ElasticNode example.

class Ball(QtGui.QGraphicsEllipseItem):
    def __init__(self, x, y, observer=None, brush_color=QtCore.Qt.red):
        w = 0.5
        h = 0.5
        super(Ball, self).__init__(-w/2.0, -h/2.0, w, h)
        self.setPos(x, y)
        self.setBrush(brush_color)

        self.setFlags(  QtGui.QGraphicsItem.ItemIsSelectable
                      | QtGui.QGraphicsItem.ItemIsMovable
                      | QtGui.QGraphicsItem.ItemIsFocusable
                      | QtGui.QGraphicsItem.ItemSendsGeometryChanges)

        self.modifiers = QtCore.Qt.NoModifier

        self.current_position = self.pos()
        self.observer = observer

        self.chains = list()

    def add_chain(self, chain):
        self.chains.append(chain)

    def mouseMoveEvent(self, event):
        self.modifiers = event.modifiers()
        super(Ball, self).mouseMoveEvent(event)

    def itemChange(self, change, value):
        if change == QtGui.QGraphicsItem.ItemPositionChange:
            if len(self.chains) != 1:
                return super(Ball, self).itemChange(change, value)
            if self.modifiers == QtCore.Qt.ShiftModifier:
                return self.restricted_to_straight_line(value.toPointF())
            if self.modifiers == QtCore.Qt.ControlModifier:
                point = self.snap_point(value.toPointF())
                if point:
                    return point
            elif self.modifiers == (QtCore.Qt.ShiftModifier | QtCore.Qt.ControlModifier):
                return self.extend_to_meet(value.toPointF())
        elif change == QtGui.QGraphicsItem.ItemPositionHasChanged:
            for chain in self.chains:
                chain.adjust()
        return super(Ball, self).itemChange(change, value)

    def mouseReleaseEvent(self, event):
        mouse_position = self.mapToScene(event.pos())
        if len(self.chains) == 1 and event.modifiers() == QtCore.Qt.ShiftModifier:
            point = self.restricted_to_straight_line(mouse_position)
        elif len(self.chains) == 1 and event.modifiers() == (  QtCore.Qt.ShiftModifier \
                                                             | QtCore.Qt.ControlModifier):
            point = self.extend_to_meet(mouse_position)
        elif event.modifiers() == QtCore.Qt.ControlModifier:
            point = self.snap_point(mouse_position)
        else:
            point = mouse_position

        if not point:
            self.setPos(self.current_position)
        else:
            self.setPos(point)
            self.current_position = self.pos()
            if self.observer:
                self.observer.notify(point.x(), point.y())
        self.modifiers = QtCore.Qt.NoModifier
        super(Ball, self).mouseReleaseEvent(event)

    def snap_point(self, position):
        point = Point(position.x(), position.y())

        w = 2
        h = 2
        for graphics_item in self.scene().items(point.x - w/2.0, point.y - h/2.0, w, h):
            if graphics_item == self:
                continue
            if graphics_item.is_selectable:
                road_item = graphics_item.road_item
                if hasattr(road_item, "position"):
                    if point.distance(road_item.position) < 1.0:
                        return road_item.position
                else:
                    for p in road_item.polyline:
                        if point.distance(p) < 1.0:
                            return QtCore.QPointF(p.x, p.y)
        return None

    def restricted_to_straight_line(self, position):
        line = self.chains[0].line()
        p1 = Point(line.x1(), line.y1())
        p2 = Point(line.x2(), line.y2())
        p = Point(position.x(), position.y())
        closest, distance = nearest_point(p1, p, p2)
        return QtCore.QPointF(closest.x, closest.y)

    def extend_to_meet(self, position):
        road_item = None
        w = 2
        h = 2
        for graphics_item in self.scene().items(position.x() - w/2.0, position.y() - h/2.0, w, h):
            if graphics_item == self:
                continue
            if not graphics_item.is_selectable:
                continue
            road_item = graphics_item.road_item
            if hasattr(road_item, "polyline"):
                break

        if not road_item:
            return self.restricted_to_straight_line(position)

        line = self.chains[0].line()
        p1 = Point(line.x1(), line.y1())
        p2 = Point(line.x2(), line.y2())
        polyline = road_item.polyline
        for i in range(len(polyline) - 1):
            p3 = polyline[i]
            p4 = polyline[i+1]
            p = intersection_point(p1, p2, p3, p4)
            if is_between(p, p3, p4):
                return QtCore.QPointF(p.x, p.y)

        return self.restricted_to_straight_line(position)

class Chain(QtGui.QGraphicsLineItem):
    def __init__(self, ball1, ball2=None, pen_color=QtCore.Qt.red):
        super(Chain, self).__init__()
        self.ball1 = ball1
        self.ball2 = ball2
        self.ball1.add_chain(self)
        if self.ball2:
            self.ball2.add_chain(self)

        self.setPen(pen_color)
        self.adjust()

    def adjust(self):
        if not self.ball2:
            return
        self.setLine(self.ball1.pos().x(), self.ball1.pos().y(),
                     self.ball2.pos().x(), self.ball2.pos().y())

    def adjust_free_end(self, x, y):
        self.setLine(self.ball1.pos().x(), self.ball1.pos().y(), x, y)

# vim:columns=100:smartindent:shiftwidth=4:expandtab:softtabstop=4:
