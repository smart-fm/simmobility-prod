#!/usr/bin/env python

# Copyright (2012) Singapore-MIT Alliance for Research and Technology

from PyQt4 import QtGui, QtCore

from paper import Tracing_paper
from digital_map import Lane_marking, Kerb_line, Road_section
from network import Lane_edge
from point import Point, nearest_point, re_arrange_co_linear_points, simplify_polyline, \
                  intersection_point, is_between
from error import show_error_message, show_info_message

class Wanderlust:
    def __init__(self, main_window):
        self.main_window = main_window
        self.tracing_paper = Tracing_paper()

    def load_digital_map(self, digital_map):
        self.load_roads(digital_map.roads)
        self.load_lane_markings(digital_map.lane_markings)
        self.load_kerb_lines(digital_map.kerb_lines)
        self.load_arrow_markings(digital_map.arrow_markings)
        self.load_bus_stops(digital_map.bus_stops)
        self.load_traffic_signals(digital_map.traffic_signals)

    def load_roads(self, roads):
        for road_name in roads:
            for road_section in roads[road_name]:
                self.tracing_paper.mark_out_polyline(road_section)

    def load_lane_markings(self, lane_markings):
        for marking_type in lane_markings:
            for lane_marking in lane_markings[marking_type]:
                self.load_lane_marking(lane_marking)

    def load_lane_marking(self, lane_marking):
        self.tracing_paper.mark_out_polyline(lane_marking)

    def load_kerb_lines(self, kerb_lines):
        for kerb_line in kerb_lines:
            self.tracing_paper.mark_out_polyline(kerb_line)

    def load_arrow_markings(self, arrow_markings):
        for arrow_marking in arrow_markings:
            self.tracing_paper.mark_out_position(arrow_marking)

    def load_bus_stops(self, bus_stops):
        for bus_stop in bus_stops:
            self.tracing_paper.mark_out_position(bus_stop)

    def load_traffic_signals(self, traffic_signals):
        for signal in traffic_signals:
            self.tracing_paper.mark_out_position(signal)

    def load_road_network(self, road_network):
        self.load_lane_edges(road_network.lane_edges)

    def load_lane_edges(self, lane_edges):
        for marking_type in lane_edges:
            for lane_edge in lane_edges[marking_type]:
                self.load_lane_edge(lane_edge)

    def load_lane_edge(self, lane_edge):
        graphics_item = self.tracing_paper.mark_out_polyline(lane_edge)
        graphics_item.setZValue(1)

################################################################################
# Extract crossings and stop-lines
################################################################################

    def extract_crossings_and_stop_lines(self, path, marking_types):
        lane_markings = list()
        for graphics_item in self.tracing_paper.items(path):
            road_item = graphics_item.road_item
            if hasattr(road_item, "lane_edge"):
                continue
            if isinstance(road_item, Lane_marking) and road_item.marking_type in marking_types:
                lane_markings.append(road_item)

        if len(lane_markings) == 0:
            show_info_message("Either there are no crossing and stop-line lane-marking in the "
                "selection region or crossing and stop-line lane-edges have already been created "
                "over the lane-markings.")
            return list()

        lane_edges = list()

        while len(lane_markings):
            group = set()
            lane_marking = lane_markings[0]
            self.transfer(lane_marking, lane_markings, group)
            self.find_similar_nearby_lane_markings(lane_marking, lane_markings, group)
            marking_type = "J" if marking_types[0] == "A4" else marking_types[0]
            lane_edge = self.form_lane_edge(group, marking_type)
            lane_edges.append(lane_edge)
        return lane_edges

    def transfer(self, lane_marking, lane_markings, group):
        if lane_marking in lane_markings:
            lane_markings.remove(lane_marking)
        group.add(lane_marking)
        self.tracing_paper.hide(lane_marking)

    def find_similar_nearby_lane_markings(self, lane_marking, lane_markings, group):
        polyline = lane_marking.polyline
        marking_type = lane_marking.marking_type
        w = h = 2
        for road_item in self.tracing_paper.road_items_around_point(polyline[0], w, h):
            if isinstance(road_item, Lane_marking) and road_item.marking_type == marking_type:
                if road_item.polyline[0].distance(road_item.polyline[-1]) < 0.1:
                    continue
                if self.is_co_linear(lane_marking, road_item):
                    self.transfer(road_item, lane_markings, group)
                    self.find_similar_nearby_lane_markings(road_item, lane_markings, group)
        for road_item in self.tracing_paper.road_items_around_point(polyline[-1], w, h):
            if isinstance(road_item, Lane_marking) and road_item.marking_type == marking_type:
                if road_item.polyline[0].distance(road_item.polyline[-1]) < 0.1:
                    continue
                if self.is_co_linear(lane_marking, road_item):
                    self.transfer(road_item, lane_markings, group)
                    self.find_similar_nearby_lane_markings(road_item, lane_markings, group)

    def is_co_linear(self, road_item1, road_item2):
        p1 = road_item1.polyline[0]
        p2 = road_item1.polyline[-1]
        discard, distance1 = nearest_point(p1, road_item2.polyline[0], p2)
        discard, distance2 = nearest_point(p1, road_item2.polyline[-1], p2)
        return (distance1 < 0.5) and (distance2 < 0.5)

    def form_lane_edge(self, group, marking_type):
        points = list()
        for road_item in group:
            for p in road_item.polyline:
                points.append(p)
        points = re_arrange_co_linear_points(points)
        polyline = simplify_polyline(points)
        lane_edge = self.main_window.road_network.add_lane_edge(marking_type, polyline)

        self.main_window.scene.draw_lane_edge(lane_edge)
        self.load_lane_edge(lane_edge)
        for road_item in group:
            self.tracing_paper.show(road_item)
            road_item.lane_edge = lane_edge

        return lane_edge

    def extend_stop_lines_to_kerb(self, stop_lines):
        for stop_line in stop_lines:
            kerb_lines = list()
            p1 = stop_line.polyline[0]
            p2 = stop_line.polyline[-1]
            # There should be only 2 kerb-lines, but if there is a road-divider, which is usually
            # only one meter wide, then the following search may be too wide and return the other
            # side of the road-divider, thus adding a third kerb-line.
            for road_item in self.tracing_paper.road_items_around_line(p1, p2, 2, 0.1):
                if isinstance(road_item, Kerb_line):
                    kerb_lines.append(road_item)

            if len(kerb_lines) < 2:
                continue
            if len(kerb_lines) > 3:
                print "found a stop-line (%s, %s) that cuts more than 3 kerb-lines" % (p1, p2)
                continue

            stop_line_polyline = list()
            for p in stop_line.polyline:
                stop_line_polyline.append(p)

            polyline = list()
            for kerb_line in kerb_lines:
                for i in range(len(kerb_line.polyline) - 1):
                    p3 = kerb_line.polyline[i]
                    p4 = kerb_line.polyline[i+1]
                    p = intersection_point(p3, p4, p1, p2)
                    if is_between(p, p3, p4):
                        # A long kerb-line may be folded like the letter 'U'.  Just like a
                        # horizontal line may intersect 'U' at 2 places, the stop-line may also
                        # intersect the kerb-line at 2 places.  We ignore the intersection that
                        # is too far away from the stop-line.
                        if p.distance(p1) < 2 or p.distance(p2) < 2:
                            polyline.append(p)
                            # The 'U'-shaped kerb-line may represent the road-divider, which is
                            # usually one meter wide, enough room for someone to stand on.  It is
                            # possible that the latter part of the kerb-line is nearer to the
                            # stop-line (we don't know which side of the 'U'-shaped kerb-line is
                            # the stop-line.  Therefore, we cannot break out of the for-loop; we
                            # have to check the other "stem" of the 'U'.

            re_arrange_co_linear_points(polyline)
            if polyline[0].distance(p1) > polyline[0].distance(p2):
                # The first point is closer to the last point of the stop-line.  We reverse the
                # polyline so that it is in the same direction as the stop-line's polyline.
                polyline.reverse()
            if len(polyline) < 2:
                continue
            if len(polyline) == 2:
                stop_line_polyline[0] = polyline[0]
                stop_line_polyline[-1] = polyline[-1]
            elif len(polyline) == 3:
                if polyline[1].distance(p1) < polyline[1].distance(p2):
                    # The middle point is closer to the stop-line's first point.
                    stop_line_polyline[0] = polyline[1]
                    stop_line_polyline[-1] = polyline[2]
                else:
                    stop_line_polyline[0] = polyline[0]
                    stop_line_polyline[-1] = polyline[1]
            else:
                print "found a stop-line (%s, %s) that cuts at more than 3 places" % (p1, p2)
                continue
            stop_line.polyline = stop_line_polyline

            point = stop_line_polyline[0]
            path = QtGui.QPainterPath(QtCore.QPointF(point.x, point.y))
            for point in stop_line_polyline:
                path.lineTo(point.x, point.y)
            stop_line.visual_item.setPath(path)

    def connect_nearby_crossings(self, crossings):
        while len(crossings):
            group = set()
            crossing = crossings[0]
            self.transfer(crossing, crossings, group)

            road_section = None
            for road_item in self.tracing_paper.road_items_intersecting_polyline(crossing.polyline):
                if isinstance(road_item, Road_section):
                    road_section = road_item
                    break
            for road_item in self.tracing_paper.road_items_intersecting_polyline(road_section.polyline):
                if isinstance(road_item, Lane_edge) and road_item.marking_type == "J":
                    self.transfer(road_item, crossings, group)

            next_road_section = None
            p1 = road_section.polyline[0]
            p2 = road_section.polyline[-1]
            for road_item in self.tracing_paper.road_items_around_line(p1, p2, 0.5, 0.5):
                if isinstance(road_item, Road_section) and road_item.road_type in ("I", "XJ", "TJ"):
                    next_road_section = road_item
                    break

            p = next_road_section.polyline[0]
            if p.distance(p1) < 0.5 or p.distance(p2) < 0.5:
                # The first point of next_road_section is closer to the previous road-section;
                # so its end-point is at the center of the intersection.
                p = next_road_section.polyline[-1]
                for road_item in self.tracing_paper.road_items_around_point(p, 0.5, 0.5):
                    if road_item == next_road_section:
                        continue
                    if isinstance(road_item, Road_section) and road_item.road_type in ("I", "XJ", "TJ"):
                        self.find_crossing_on_other_side(road_item, p, crossings, group)
            else:
                # The first point of next_road_section is at the center of the intersection.
                for road_item in self.tracing_paper.road_items_around_point(p, 0.5, 0.5):
                    if road_item == next_road_section:
                        continue
                    if isinstance(road_item, Road_section) and road_item.road_type in ("I", "XJ", "TJ"):
                        self.find_crossing_on_other_side(road_item, p, crossings, group)

    def find_crossing_on_other_side(self, road_section, point, crossings, group):
        if point.distance(road_section.polyline[0]) < 0.5:
            p = road_section.polyline[-1]
        else:
            p = road_section.polyline[0]
        next_road_section = None
        for road_item in self.tracing_paper.road_items_around_point(p, 0.5, 0.5):
            if road_item == road_section:
                continue
            if isinstance(road_item, Road_section):
                next_road_section = road_item
                break

        road_section = next_road_section
        for road_item in self.tracing_paper.road_items_intersecting_polyline(road_section.polyline):
            if isinstance(road_item, Lane_edge) and road_item.marking_type == "J":
                self.transfer(road_item, crossings, group)

################################################################################
# Extract yellow boxes
################################################################################

    def extract_yellow_boxes(self, path):
        lane_markings = list()
        for graphics_item in self.tracing_paper.items(path):
            road_item = graphics_item.road_item
            if hasattr(road_item, "lane_edge"):
                continue
            if isinstance(road_item, Lane_marking) and road_item.marking_type == "N":
                lane_markings.append(road_item)

        if len(lane_markings) == 0:
            show_info_message("Either there are no yellow-box lane-marking in the selection "
                "region or a yellow-box lane-edge has already been created over the lane-markings.")

        while len(lane_markings):
            group = set()
            lane_marking = lane_markings[0]
            self.transfer(lane_marking, lane_markings, group)
            self.find_connected_yellow_box(lane_marking, lane_markings, group)
            self.form_yellow_box(group)

    def find_connected_yellow_box(self, lane_marking, lane_markings, group):
        for point in lane_marking.polyline:
            for road_item in self.tracing_paper.road_items_around_point(point, 0.2, 0.2):
                if isinstance(road_item, Lane_marking) and road_item.marking_type == "N":
                    self.transfer(road_item, lane_markings, group)
                    self.find_connected_yellow_box(road_item, lane_markings, group)

    def form_yellow_box(self, group):
        scene_rect = self.main_window.scene.sceneRect()
        left = scene_rect.right()
        right = scene_rect.left()
        top = scene_rect.top()
        bottom = scene_rect.bottom()

        north = None
        east = None
        west = None
        south = None
        for road_item in group:
            for point in road_item.polyline:
                if left > point.x:
                    left = point.x
                    west = point
                if right < point.x:
                    right = point.x
                    east = point
                if bottom > point.y:
                    bottom = point.y
                    south = point
                if top < point.y:
                    top = point.y
                    north = point

        polyline = list()
        polyline.append(north)
        polyline.append(east)
        polyline.append(south)
        polyline.append(west)
        polyline.append(north)
        polyline.append(south)
        polyline.append(east)
        polyline.append(west)

        lane_edge = self.main_window.road_network.add_lane_edge("N", polyline)
        self.main_window.scene.draw_lane_edge(lane_edge)
        self.load_lane_edge(lane_edge)
        for road_item in group:
            self.tracing_paper.show(road_item)
            road_item.lane_edge = lane_edge

################################################################################
# Create missing stop-line
################################################################################

    def create_missing_stop_line(self, path):
        crossings, lane_markings, kerb_lines = self.items_in_selection_region(path)
        if len(kerb_lines) < 2:
            show_error_message("The selection region must include at least 2 kerb-lines.")
            return

        important_marking_types = ("B", "C", "A", "L", "S", "S1", "A1")
        for lane_marking in lane_markings:
            if lane_marking.marking_type in important_marking_types:
                break
        else:
            msg = "The selection region must contain at least one lane-marking of the types: "
            msg = msg + "'%s'" % important_marking_types[0]
            for type in important_marking_types[1:]:
                msg = msg + ", '%s'" % type
            show_error_message(msg)
            return

        crossing = crossings[0]
        ref_point = self.get_reference_point(lane_marking, crossing)
        stop_line = self.calc_stop_line(ref_point, crossing, kerb_lines)

        for lane_marking in lane_markings:
            polyline = self.get_polyline(lane_marking, crossing)
            p1 = polyline[-1]
            p2 = polyline[-2]
            p = intersection_point(p1, p2, stop_line[0], stop_line[-1])
            if is_between(p, stop_line[0], stop_line[-1]) and is_between(p, p1, p2):
                polyline[-1] = p
                self.adjust_lane_marking(lane_marking, polyline)

        lane_marking = self.main_window.digital_map.add_lane_marking("M", "discard", stop_line)
        self.main_window.scene.draw_lane_marking(lane_marking)
        self.load_lane_marking(lane_marking)

    def items_in_selection_region(self, path):
        crossings = list()
        lane_markings = list()
        kerb_lines = list()

        for graphics_item in self.tracing_paper.items(path):
            road_item = graphics_item.road_item
            if isinstance(road_item, Lane_marking):
                if road_item.marking_type in ("J", "A4"):
                    crossings.append(road_item)
                else:
                    lane_markings.append(road_item)
            elif isinstance(road_item, Kerb_line):
                kerb_lines.append(road_item)
        return crossings, lane_markings, kerb_lines

    def get_reference_point(self, lane_marking, crossing):
        polyline = self.get_polyline(lane_marking, crossing)

        p1 = polyline[-1]
        p2 = polyline[-2]
        p3 = crossing.polyline[0]
        p4 = crossing.polyline[-1]
        p = intersection_point(p1, p2, p3, p4)

        vec = p2 - p
        p = p + (2.5 / abs(vec)) * vec
        return p

    def get_polyline(self, lane_marking, crossing):
        polyline = simplify_polyline(lane_marking.polyline)

        point = crossing.polyline[0]
        if point.distance(polyline[0]) < point.distance(polyline[-1]):
            # The first point of the lane-marking polyline is closer to the crossing.  This is not
            # correct; the polyline should be moving towards the stop-line/crossing.
            polyline.reverse()

        return polyline

    def calc_stop_line(self, ref_point, crossing, kerb_lines):
        stop_line = list()

        vec = crossing.polyline[-1] - crossing.polyline[0]
        p3 = ref_point
        p4 = p3 + vec

        for kerb_line in kerb_lines:
            for i in range(len(kerb_line.polyline) - 1):
                p1 = kerb_line.polyline[i]
                p2 = kerb_line.polyline[i+1]
                p = intersection_point(p1, p2, p3, p4)
                if is_between(p, p1, p2):
                    stop_line.append(p)
                    break

        if len(stop_line) < 2:
            raise RuntimeError("stop-line has only %d point" % len(stop_len))
        if len(stop_line) == 2:
            return stop_line

        stop_line.sort(key = lambda point : point.x)
        if is_between(ref_point, stop_line[0], stop_line[1]):
            return (stop_line[0], stop_line[1])
        return (stop_line[1], stop_line[2])

    def adjust_lane_marking(self, lane_marking, polyline):
        lane_marking.polyline = polyline
        point = polyline[0]
        path = QtGui.QPainterPath(QtCore.QPointF(point.x, point.y))
        for point in polyline[1:]:
            path.lineTo(point.x, point.y)
        lane_marking.visual_item.setPath(path)

# vim:columns=100:smartindent:shiftwidth=4:expandtab:softtabstop=4:
