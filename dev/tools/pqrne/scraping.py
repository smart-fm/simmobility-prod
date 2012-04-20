#!/usr/bin/env python

# Copyright (2012) Singapore-MIT Alliance for Research and Technology

from PyQt4 import QtGui, QtCore

from point import Point, intersection_point, is_between, angle_between_2_lines, simplify_polyline
from chain import Ball, Chain
from network import Lane_edge
from edit import Edit_form

"""A Scraping is the temporary object created when the user "strings" lane-markings or
kerb-line points together to form a lane edge."""

def set_direction(polyline, point, is_moving_away=True):
    """If necessary, change the direction of @polyline@ so that it is moving away from @point@.
    Return true if the polyline has to change direction."""
    d1 = polyline[0].manhattan_distance(point)
    d2 = polyline[1].manhattan_distance(point)
    if d2 > d1:
        if is_moving_away:
            return False
        else:
            polyline.reverse()
            return True
    else:
        if is_moving_away:
            polyline.reverse()
            return True
        else:
            return False

class Scraping(QtCore.QObject):
    # The user can add the entire lane-marking or individual points (usually of the kerb-line)
    # to the Scraping.
    def __init__(self, main_window):
        super(Scraping, self).__init__()
        self.main_window = main_window
        self.polyline = list()
        self.road_items = list()
        self.graphics_items = list()
        self.direction_is_confirmed = True
        self.chain = None

    def add(self, point_item, line_item, lane_edge_item, snap_to_line):
        if lane_edge_item:
            if self.extend_to_meet(lane_edge_item):
                return True
        elif line_item:
            if self.extend_to_meet(line_item):
                return True

        if snap_to_line:
            if line_item:
                self.add_line_item(line_item)
            return True
        else:
            if point_item:
                self.add_point_item(point_item)
            return True

        return False

    def add_point_item(self, graphics_item):
        self.add_road_item(graphics_item)

        pos = graphics_item.pos()
        self.add_point(Point(pos.x(), pos.y()))

    def add_line_item(self, graphics_item):
        self.add_road_item(graphics_item)

        polyline = graphics_item.road_item.polyline
        if len(self.polyline) == 0:
            for point in polyline:
                self.add_point(point)
            self.direction_is_confirmed = False
            return

        if not self.direction_is_confirmed:
            self.set_both_directions(polyline)
        else:
            set_direction(polyline, self.polyline[-1])
        for point in polyline:
            self.add_point(point)

    def extend_to_meet(self, graphics_item):
        if len(self.polyline) < 2:
            return False

        polyline = graphics_item.road_item.polyline
        p = polyline[0]
        if polyline[0].distance(self.polyline[0]) < polyline[0].distance(self.polyline[-1]):
            p3 = self.polyline[0]
            p4 = self.polyline[1]
            for i in range(len(polyline) - 1):
                p1 = polyline[i]
                p2 = polyline[i+1]
                p = intersection_point(p1, p2, p3, p4)
                if is_between(p, p1, p2):
                    angle = angle_between_2_lines(p1, p, p4)
                    if angle > 30 and angle < 150:
                        if p3.distance(p) < 3:
                            self.draw_line(self.polyline[0], p)
                            self.polyline[0] = p
                            return True
        else:
            p3 = self.polyline[-1]
            p4 = self.polyline[-2]
            for i in range(len(polyline) - 1):
                p1 = polyline[i]
                p2 = polyline[i+1]
                p = intersection_point(p1, p2, p3, p4)
                if is_between(p, p1, p2):
                    angle = angle_between_2_lines(p1, p, p4)
                    if angle > 30 and angle < 150:
                        if p3.distance(p) < 3:
                            self.draw_line(self.polyline[-1], p)
                            self.polyline[-1] = p
                            return True
        return False

    def adjust(self, x, y):
        if self.chain:
            self.chain.adjust_free_end(x, y)

    def finish(self):
        self.main_window.scene.removeItem(self.chain)
        for graphics_item in self.road_items:
            graphics_item.is_selectable = True
        for graphics_item in self.graphics_items:
            self.main_window.scene.removeItem(graphics_item)

        if len(self.polyline) < 2:
            return

        polyline = simplify_polyline(self.polyline)
        self.lane_edge = self.main_window.road_network.add_lane_edge("e", polyline)
        self.main_window.scene.draw_lane_edge(self.lane_edge)

        self.lane_edge.visual_item.setPen(QtCore.Qt.red)
        self.edit_form = Edit_form(self.lane_edge, self.main_window)
        self.connect(self.edit_form, QtCore.SIGNAL("finish_editing"), self.finish_editing)
        self.edit_form.setVisible(True)

    def finish_editing(self):
        #if self.edit_form.result() == QtGui.QDialog.DialogCode.Accepted:
        if self.edit_form.result() == QtGui.QDialog.Accepted:
            self.lane_edge.visual_item.setPen(QtCore.Qt.blue)
            self.main_window.wanderlust.load_lane_edge(self.lane_edge)
        else:
            self.main_window.scene.removeItem(self.lane_edge.visual_item)
            self.main_window.road_network.expunge(self.lane_edge)
        self.emit(QtCore.SIGNAL("finish_scraping"))
        self.edit_form = None

    def add_road_item(self, graphics_item):
        self.road_items.append(graphics_item)
        graphics_item.is_selectable = False

    def add_point(self, point):
        if not self.direction_is_confirmed:
            self.determine_direction(point)

        if len(self.polyline):
            self.draw_line(self.polyline[-1], point)
        self.polyline.append(point)

        self.set_ball_and_chain()

    def determine_direction(self, point):
        if set_direction(self.polyline, point):
            pass
        self.direction_is_confirmed = True

    def set_both_directions(self, polyline):
        if set_direction(self.polyline, polyline[0], False):
            pass
        set_direction(polyline, self.polyline[-1])
        self.direction_is_confirmed = True

    def draw_line(self, p1, p2):
        path = QtGui.QPainterPath(QtCore.QPointF(p1.x, p1.y))
        path.lineTo(p2.x, p2.y)
        graphics_item = QtGui.QGraphicsPathItem(path)
        graphics_item.setPen(QtCore.Qt.red)
        graphics_item.is_selectable = False
        self.graphics_items.append(graphics_item)
        self.main_window.scene.addItem(graphics_item)

    def set_ball_and_chain(self):
        if len(self.polyline) == 1:
            point = self.polyline[0]
            self.ball = Ball(point.x, point.y)
            #self.ball_is_selectable = False
            # Do not add the ball to the graphics_scene so that it is not shown to prevent the
            # user from moving it.
            self.chain = Chain(self.ball, None)
            self.chain.is_selectable = False
            self.main_window.scene.addItem(self.chain)
            return
        self.set_fixed_end(self.polyline[-1])

    def set_fixed_end(self, point):
        self.ball.setPos(point.x, point.y)

# vim:columns=100:smartindent:shiftwidth=4:expandtab:softtabstop=4:
