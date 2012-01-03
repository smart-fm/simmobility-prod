#!/usr/bin/python

# Copyright Singapore-MIT Alliance for Research and Technology

from PyQt4 import QtGui, QtCore

# Based on the Qt4 ElasticNode example, which is also bundled with PyQt4.

"""This module defines the Ball and Chain classes which are PyQt4 graphics items for rendering the points and lines of a lane edge.  Each Ball object is connected to another Ball object by a Chain object which is rendered as a line between the 2 Ball objects.  A Ball object can be dragged around by the user in a view.  As the user moves the Ball object around, the line(s) representing the Chain object(s) are redrawn."""

class Ball(QtGui.QGraphicsEllipseItem):
    balls = list()
    balls_are_hidden = False

    @staticmethod
    def toggle_ball_visibility():
        if not Ball.balls_are_hidden:
            for ball in Ball.balls:
                ball.hide()
            Ball.balls_are_hidden = True
        else:
            for ball in Ball.balls:
                ball.show()
            Ball.balls_are_hidden = False

    def __init__(self, x, y, brush=QtCore.Qt.red):
        w = 50
        h = 50
        super(Ball, self).__init__(-w / 2, -h / 2, w, h)
        self.setPos(x, y)
        self.setBrush(brush)

        self.setFlags(  QtGui.QGraphicsItem.ItemIsSelectable
                      | QtGui.QGraphicsItem.ItemIsMovable
                      | QtGui.QGraphicsItem.ItemIsFocusable
                      | QtGui.QGraphicsItem.ItemSendsGeometryChanges)
        self.is_moving = False

        self.chains = list()

        Ball.balls.append(self)

    def set_color(self, color):
        self.setBrush(color)

    def add_chain(self, chain):
        self.chains.append(chain)

    def itemChange(self, change, value):
        if QtGui.QGraphicsItem.ItemPositionHasChanged == change:
            for chain in self.chains:
                chain.adjust()
            self.is_moving = True
        return super(Ball, self).itemChange(change, value)

    def mouseReleaseEvent(self, event):
        if self.is_moving:
            self.is_moving = False
            point = self.mapToScene(event.pos())
            self.lane_edge.polyline[self.point_index].x = point.x()
            self.lane_edge.polyline[self.point_index].y = point.y()
        super(Ball, self).mouseReleaseEvent(event)

    def get_siblings(self, siblings):
        for chain in self.chains:
            if chain not in siblings:
                siblings.append(chain)
                chain.get_siblings(siblings)

class Chain(QtGui.QGraphicsLineItem):
    def __init__(self, ball1, ball2=None, pen=QtCore.Qt.red):
        super(Chain, self).__init__()
        self.ball1 = ball1
        self.ball2 = ball2
        self.ball1.add_chain(self)
        if self.ball2:
            self.ball2.add_chain(self)

        self.setPen(pen)
        self.adjust()

    def set_color(self, color):
        self.setPen(color)

    def adjust(self):
        if not self.ball2:
            return
        self.setLine(self.ball1.pos().x(), self.ball1.pos().y(),
                     self.ball2.pos().x(), self.ball2.pos().y())

    def adjust_free_end(self, x, y):
        self.setLine(self.ball1.pos().x(), self.ball1.pos().y(), x, y)

    def get_siblings(self, siblings):
        if self.ball1:
            if self.ball1 not in siblings:
                siblings.append(self.ball1)
                self.ball1.get_siblings(siblings)
        if self.ball2:
            if self.ball2 not in siblings:
                siblings.append(self.ball2)
                self.ball2.get_siblings(siblings)
