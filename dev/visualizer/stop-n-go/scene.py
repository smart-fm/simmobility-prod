#!/usr/bin/env python

# Copyright Singapore-MIT Alliance for Research and Technology

from PyQt4 import QtGui, QtCore

class Graphics_scene(QtGui.QGraphicsScene):
    def __init__(self, parent=None):
        super(Graphics_scene, self).__init__(parent)

    def draw_road_network(self, road_network):
        for segment in road_network.road_segments:
            for lane_edge in segment.lane_edges.values():
                point = lane_edge.points[0]
                path = QtGui.QPainterPath(QtCore.QPointF(point.x, point.y))
                for point in lane_edge.points[1:]:
                    path.lineTo(point.x, point.y)
                item = QtGui.QGraphicsPathItem(path)
                self.addItem(item)
        for crossing in road_network.crossings:
            point1 = crossing.near[0]
            point2 = crossing.near[1]
            item = QtGui.QGraphicsLineItem(point1.x, point1.y, point2.x, point2.y)
            self.addItem(item)
            point1 = crossing.far[0]
            point2 = crossing.far[1]
            item = QtGui.QGraphicsLineItem(point1.x, point1.y, point2.x, point2.y)
            self.addItem(item)
