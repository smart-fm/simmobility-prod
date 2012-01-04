#!/usr/bin/env python

# Copyright Singapore-MIT Alliance for Research and Technology

from PyQt4 import QtGui, QtCore

from chain import Ball, Chain

class Graphics_scene(QtGui.QGraphicsScene):
    def __init__(self, parent=None):
        super(Graphics_scene, self).__init__(parent)
        self.node_items = list()
        self.nodes_are_hidden = True

    def draw_road_network(self, road_network, tool_box):
        self.draw_nodes(road_network)
        self.draw_lane_markings(road_network, tool_box)
        self.draw_kerb_lines(road_network, tool_box)
        self.draw_lane_edges(road_network, tool_box)
        self.draw_center_lines(road_network, tool_box)

    def draw_nodes(self, road_network):
        for node in road_network.nodes.values():
            info = "%s id=%d" % ("multi-node" if node.is_intersection else "uni-node", node.id)
            #for section in node.from_sections:
            #    info += " from-section=%d" % section.id
            #for section in node.to_sections:
            #    info += " to-section=%d" % section.id
            item = self.draw_point(node, color=QtCore.Qt.red, info=info, large=True)
            item.hide()
            self.node_items.append(item)

    def toggle_node_visibility(self):
        if self.nodes_are_hidden:
            for item in self.node_items:
                item.show()
            self.nodes_are_hidden = False
        else:
            for item in self.node_items:
                item.hide()
            self.nodes_are_hidden = True

    def draw_lane_markings(self, road_network, tool_box):
        for section in road_network.sections.values():
            for lane_marking in section.lane_markings.values():
                group = tool_box.lane_marking_type(lane_marking.type)
                info =   "lane-marking id=%d section=%d type=%s" \
                       % (lane_marking.id, lane_marking.section_id, lane_marking.type)
                item = self.draw_polyline(lane_marking.polyline, group, info, None)
                item.road_item = lane_marking

    def draw_kerb_lines(self, road_network, tool_box):
        for section in road_network.sections.values():
            for kerb_line in section.kerb_lines.values():
                group = tool_box.kerb_line_type(kerb_line.type)
                info =   "kerb-line id=%d section=%d type=%s" \
                       % (kerb_line.id, kerb_line.section_id, kerb_line.type)
                item = self.draw_polyline(kerb_line.polyline, group, info, None)
                item.road_item = kerb_line

    def draw_lane_edges(self, road_network, tool_box):
        for lane_edge in road_network.lane_edges:
            self.draw_lane_edge(lane_edge, tool_box)

    def draw_lane_edge(self, lane_edge, tool_box):
        color = QtCore.Qt.blue
        group = tool_box.lane_edge_type(lane_edge.type)
        info = "lane-edge section=%d index=%d type=%s" \
               % (lane_edge.section_id, lane_edge.index, lane_edge.type)
        balls = list()
        for i, point in enumerate(lane_edge.polyline):
            ball = Ball(point.x, point.y, color)
            if i:
                ball.info = info
            else:
                ball.info = info + "polyline-first-point"
            ball.road_item = lane_edge
            ball.lane_edge = lane_edge
            ball.point_index = i
            ball.setZValue(1)
            ball.is_a_line = False
            ball.is_selectable = True
            self.addItem(ball)
            balls.append(ball)
            group.items.append(ball)

        ball0 = balls[0]
        for ball in balls:
            chain = Chain(ball0, ball, color)
            ball0 = ball
            chain.info = info
            chain.road_item = lane_edge
            chain.lane_edge = lane_edge
            chain.setZValue(1)
            chain.is_a_line = True
            chain.is_selectable = True
            self.addItem(chain)
            group.items.append(chain)

    def draw_center_lines(self, road_network, tool_box):
        for center_line in road_network.lane_center_lines:
            self.draw_center_line(center_line, tool_box)

    def draw_center_line(self, center_line, tool_box):
        color = QtGui.QColor("gray")
        group = tool_box.lane_edge_type(center_line.type)
        info = "center-line section=%d index=%d type=%s" \
               % (center_line.section_id, center_line.index, center_line.type)
        self.draw_balls_and_chains(center_line, info, group, color)

    def draw_balls_and_chains(self, lane_edge_or_center_line, info, group, color):
        balls = list()
        for i, point in enumerate(lane_edge_or_center_line.polyline):
            ball = Ball(point.x, point.y, color)
            if i:
                ball.info = info
            else:
                ball.info = info + "polyline-first-point"
            ball.road_item = object
            ball.lane_edge = object
            ball.point_index = i
            ball.setZValue(1)
            ball.is_a_line = False
            ball.is_selectable = True
            self.addItem(ball)
            balls.append(ball)
            group.items.append(ball)

        ball0 = balls[0]
        for ball in balls:
            chain = Chain(ball0, ball, color)
            ball0 = ball
            chain.info = info
            chain.road_item = lane_edge_or_center_line
            chain.lane_edge = lane_edge_or_center_line
            chain.setZValue(1)
            chain.is_a_line = True
            chain.is_selectable = True
            self.addItem(chain)
            group.items.append(chain)

    def draw_point(self, point, group=None, color=QtCore.Qt.black, info=None,
                   first_point=False, large=False):
        w = 50
        h = 50
        if first_point:
            info += " polyline-first-point"
        elif large:
            w *= 4
            h *= 4
        item = QtGui.QGraphicsEllipseItem(point.x - w / 2, point.y - h / 2, w, h)
        item.setBrush(color)
        item.info = info
        item.road_item = point
        item.is_a_line = False
        item.is_selectable = True
        if group:
            group.items.append(item)
        self.addItem(item)
        return item

    def draw_polyline(self, polyline, group, info, items, color=QtCore.Qt.black):
        if len(polyline) == 0:
            return

        point = polyline[0]
        item = self.draw_point(point, group, color, info, True)
        if isinstance(items, list):
            items.append(item)
        path = QtGui.QPainterPath(QtCore.QPointF(point.x, point.y))
        for point in polyline[1:]:
            item = self.draw_point(point, group, color, info)
            if isinstance(items, list):
                items.append(item)
            path.lineTo(point.x, point.y)
        item = QtGui.QGraphicsPathItem(path)
        if isinstance(items, list):
            items.append(item)
        item.setPen(color)
        item.info = info
        item.is_a_line = True
        item.is_selectable = True
        if group:
            group.items.append(item)
        self.addItem(item)
        return item

    def draw_line(self, line, group=None, color=QtCore.Qt.black, info=None):
        item = QtGui.QGraphicsLineItem(line[0].x, line[0].y, line[1].x, line[1].y)
        item.setPen(color)
        item.info = info
        item.is_a_line = True
        item.is_selectable = True
        if group:
            group.items.append(item)
        self.addItem(item)
        return item
