#!/usr/bin/env python

# Copyright Singapore-MIT Alliance for Research and Technology

from PyQt4 import QtGui, QtCore

from chain import Ball, Chain
import edge
from simmob.point import Point

def bisect(cut_line, road_network, view):
    # We are using a QGraphicsScene here, not for viewing but to use its spatial indexing to
    # locate nearby road items.
    digital_map = QtGui.QGraphicsScene()
    for lane_edge in road_network.lane_edges:
        add_polyline(lane_edge, digital_map)
    for center_line in road_network.lane_center_lines:
        add_polyline(center_line, digital_map)

    polyline = cut_line.polyline
    point = polyline[0]
    search_path = QtGui.QPainterPath(QtCore.QPointF(point.x, point.y))
    point = polyline[-1]
    search_path.lineTo(point.x, point.y)
    edges = list()
    balls_and_chains = dict()
    for item in digital_map.items(search_path, QtCore.Qt.IntersectsItemShape):
        if cut_line == item.road_item:
            continue
        lane_edge = item.road_item
        edges.append(lane_edge)
        collection = list()
        highlight(lane_edge, view, collection)
        balls_and_chains[lane_edge] = collection

    reply = QtGui.QMessageBox.question(None, "pqrne bisect tool",
                                       "Is it OK to bisect the green lane edges and center lines?" \
                                       + "\nNote that this operation can't be undone.",
                                       QtGui.QMessageBox.Ok | QtGui.QMessageBox.Cancel,
                                       QtGui.QMessageBox.Cancel)
    unhighlight(balls_and_chains)
    if QtGui.QMessageBox.Ok == reply:
        cut(cut_line.polyline, edges, road_network, balls_and_chains, view)

def add_polyline(lane_edge_or_center_line, digital_map):
    polyline = lane_edge_or_center_line.polyline
    point = polyline[0]
    path = QtGui.QPainterPath(QtCore.QPointF(point.x, point.y))
    for point in polyline[1:]:
        path.lineTo(point.x, point.y)
    item = QtGui.QGraphicsPathItem(path)
    item.road_item = lane_edge_or_center_line
    digital_map.addItem(item)

def highlight(road_item, view, balls_and_chains):
    scene = view.scene()
    for item in scene.items():
        if item.road_item == road_item:
            balls_and_chains.append(item)
            if isinstance(item, Chain):
                item.setPen(QtCore.Qt.green)
                item.show()
            if isinstance(item, Ball):
                item.setBrush(QtCore.Qt.green)
                item.show()

def unhighlight(balls_and_chains):
    for lane_edge, collection in balls_and_chains.iteritems():
        for ball_or_chain in collection:
            if isinstance(lane_edge, edge.Lane_edge):
                if isinstance(ball_or_chain, Chain):
                    ball_or_chain.setPen(QtCore.Qt.blue)
                else:
                    ball_or_chain.setBrush(QtCore.Qt.blue)
            elif isinstance(lane_edge, edge.Center_line):
                if isinstance(ball_or_chain, Chain):
                    ball_or_chain.setPen(QtGui.QColor("gray"))
                else:
                    ball_or_chain.setBrush(QtGui.QColor("gray"))

def cut(cut_line, edges, road_network, balls_and_chains, view):
    for lane_edge in edges:
        index, intersection_point = intersection(lane_edge.polyline, cut_line)

        new_edge = split(lane_edge, index, intersection_point, road_network)

        collection = balls_and_chains[lane_edge]
        for ball_or_chain in collection:
            view.scene().removeItem(ball_or_chain)

        if isinstance(lane_edge, edge.Lane_edge):
            view.scene().draw_lane_edge(lane_edge, view.tool_box)
            view.scene().draw_lane_edge(new_edge, view.tool_box)
        if isinstance(lane_edge, edge.Center_line):
            view.scene().draw_center_line(lane_edge, view.tool_box)
            view.scene().draw_center_line(new_edge, view.tool_box)

def intersection(polyline, cut_line):
    # The following intersection algorithm is described in Section 5.1.9.1 of "Real-Time Collision
    # Detection" by Christer Ericson, Morgan Kaufmann Publishers.
    pt1 = cut_line[0]
    pt2 = cut_line[-1]
    for i in range(len(polyline) - 1):
        pt3 = polyline[i]
        pt4 = polyline[i+1]
        area1 = triangle_area(pt1, pt2, pt4)
        area2 = triangle_area(pt1, pt2, pt3)
        if area1 != 0 and area2 != 0 and area1 * area2 < 0:
            area3 = triangle_area(pt3, pt4, pt1)
            area4 = area3 + area2 - area1
            if area3 * area4 < 0:
                t = area3 / (area3 - area4)
                x = pt1.x + t * (pt2.x - pt1.x)
                y = pt1.y + t * (pt2.y - pt1.y)
                point = Point(x, y)
                return (i, point)

def triangle_area(pt1, pt2, pt3):
    return (pt1.x - pt3.x) * (pt2.y - pt3.y) - (pt1.y - pt3.y) * (pt2.x - pt3.x) 

def split(lane_edge, last_index, new_point, road_network):
    entire_polyline = lane_edge.polyline

    polyline = list()
    polyline.extend(entire_polyline[:last_index + 1])
    polyline.append(new_point)
    lane_edge.polyline = polyline

    polyline = list()
    polyline.append(new_point)
    polyline.extend(entire_polyline[last_index + 1:])
    if isinstance(lane_edge, edge.Lane_edge):
        new_edge = edge.Lane_edge(polyline, lane_edge.type, lane_edge)
        road_network.add_lane_edge(new_edge)
    elif isinstance(lane_edge, edge.Center_line):
        new_edge = edge.Center_line(polyline, lane_edge)
        road_network.add_center_line(new_edge)
    return new_edge
