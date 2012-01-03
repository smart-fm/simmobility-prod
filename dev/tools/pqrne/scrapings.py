#!/usr/bin/python

# Copyright Singapore-MIT Alliance for Research and Technology

from PyQt4 import QtGui, QtCore

from chain import Ball, Chain
from edge import Lane_edge
import dialog

def manhattan_distance(p1, p2):
    return abs(p1.x - p2.x) + abs(p1.y - p2.y)

# If necessary, change the direction of <polyline> so that it is moving away from <point>.
# Return true if the polyline has to change direction.
def set_direction(polyline, point, is_moving_away=True):
    d1 = manhattan_distance(polyline[0], point)
    d2 = manhattan_distance(polyline[1], point)
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

class Scrapings:
    def __init__(self, graphics_scene):
        self.graphics_scene = graphics_scene
        self.polyline = list()
        self.point_items = list()
        self.line_items = list()
        self.direction_is_confirmed = True
        self.ball = None
        self.chain = None

    def add_point(self, point):
        if not self.direction_is_confirmed:
            self.determine_direction(point)

        self.polyline.append(point)
        info = "scrapings"
        graphics_item = self.graphics_scene.draw_point(point, color=QtCore.Qt.red, info=info)
        graphics_item.is_selectable = False
        self.point_items.append(graphics_item)

        self.add_line()
        self.set_ball_and_chain()

    def add_polyline(self, polyline):
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

    def add_line(self):
        if len(self.polyline) == 1:
            return

        info = "scrapings"
        graphics_item = self.graphics_scene.draw_line(self.polyline[-2:],
                                                      color=QtCore.Qt.red, info=info)
        graphics_item.is_selectable = False
        self.line_items.append(graphics_item)

    def determine_direction(self, point):
        if set_direction(self.polyline, point):
            self.point_items.reverse()
        self.direction_is_confirmed = True

    def set_both_directions(self, polyline):
        if set_direction(self.polyline, polyline[0], False):
            self.point_items.reverse()
        set_direction(polyline, self.polyline[-1])
        self.direction_is_confirmed = True

    def set_ball_and_chain(self):
        if len(self.polyline) == 1:
            point = self.polyline[0]
            self.ball = Ball(point.x, point.y)
            self.ball.is_selectable = False
            #self.graphics_scene.addItem(self.ball)
            self.chain = Chain(self.ball, None)
            self.chain.is_selectable = False
            self.graphics_scene.addItem(self.chain)
            return
        self.set_fixed_end(self.polyline[-1])

    def set_fixed_end(self, point):
        self.ball.setPos(point.x, point.y)

    def adjust(self, x, y):
        self.chain.adjust_free_end(x, y)

    def finish(self):
        #self.graphics_scene.removeItem(self.ball)
        self.graphics_scene.removeItem(self.chain)

        edge = Lane_edge(self.polyline, 'e')
        lane_edge_dialog = dialog.Dialogs.lane_edge_dialog
        lane_edge_dialog.set_lane_edge(edge)
        result = None
        if lane_edge_dialog.exec_():
            lane_edge_dialog.update(edge)
            result = edge

        for item in self.point_items:
            self.graphics_scene.removeItem(item)
        for item in self.line_items:
            self.graphics_scene.removeItem(item)

        return result
