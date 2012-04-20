#!/usr/bin/env python

# Copyright (2012) Singapore-MIT Alliance for Research and Technology

import elixir
import sqlalchemy
from sqlalchemy.ext.associationproxy import AssociationProxy
from sqlalchemy.ext.orderinglist import ordering_list
import struct
from osgeo import ogr

from point import Point
from digital_map import Digital_map

__metadata__ = sqlalchemy.schema.ThreadLocalMetaData()
__session__ = sqlalchemy.orm.scoped_session(sqlalchemy.orm.sessionmaker())

class Road_network:
    lane_edge_types = { "J" : "J - White Line-Ped Crossing",
                        "M" : "M - White Line-stop line",
                        "N" : "N - Yellow Box Line",
                        "R" : "R - Bus Zone",
                        "e" : "e - lane-edge in SimMobility",
                        "k" : "k - kerb-line in SimMobility",
                        "c" : "c - lane's center-line in SimMobility" }

    def __init__(self, database_name):
        self.engine = sqlalchemy.create_engine(database_name)
        __session__.configure(autoflush=True, bind=self.engine)
        __metadata__.bind = self.engine

        self.lane_edges = dict()

    def is_dirty(self):
        return __session__.new or __session__.dirty or __session__.deleted

    def commit_to_database(self):
        __session__.commit()

    def expunge(self, item):
        __session__.expunge(item)

    def add_lane_edge(self, marking_type, polyline):
        lane_edge = Lane_edge(marking_type=marking_type)
        lane_edge.polyline = polyline
        if marking_type not in self.lane_edges:
            self.lane_edges[marking_type] = list()
        self.lane_edges[marking_type].append(lane_edge)
        return lane_edge

    def load_database(self):
        for lane_edge in Lane_edge.query.all():
            if lane_edge.marking_type not in self.lane_edges:
                self.lane_edges[lane_edge.marking_type] = list()
            self.lane_edges[lane_edge.marking_type].append(lane_edge)

class Road_item_with_polyline:
    def __get_polyline(self):
        if not hasattr(self, "polyline__"):
            self.polyline__ = convert_from_wkb_polyline(self.database_polyline)
        return self.polyline__
    def __set_polyline(self, polyline):
        self.polyline__ = polyline
        self.database_polyline = convert_to_wkb_polyline(polyline)
    polyline = property(__get_polyline, __set_polyline)

class Road_item_at_a_point:
    def __get_position(self):
        if not hasattr(self, "position__"):
            self.position__ = convert_from_wkb_point(self.database_position)
        return self.position__
    def __set_position(self, position):
        self.position__ = position
        self.database_position = convert_to_wkb_point(position)
    position = property(__get_position, __set_position)

class Lane_edge(elixir.Entity, Road_item_with_polyline):
    """A lane-edge is one of the 2 lines that mark out the boundary of a lane.  It can also be
    a polyline that "connects" a pedestrian crossing to the adjacent pedestrian crossing so that
    the road-network looks "pretty" in the visualizer.  A lane-edge can also be a yellow-box or
    a bus zone.
    Lane-edges should only be useful to the visualizer and not the simulator unless the driver
    needs to know the outer edges of each lane."""
    elixir.using_options(tablename="lane_edges")
    marking_type = elixir.Field(elixir.Text)
    database_polyline = elixir.Field(elixir.Binary)

    def __repr__(self):
        return "lane-edge type='%s'" % (self.marking_type)

    def road_info(self):
        info =   "lane-edge={\n" \
               + "    marking-type='%s' (%s)\n" % (self.marking_type,
                                                   Road_network.lane_edge_types[self.marking_type]) \
               + "    polyline={\n"
        for i, point in enumerate(self.polyline):
            info = info + "        %4d   %s\n" % (i, point)
        info = info + "    }\n}\n"
        return info

# Please ensure the following symbolic constants have the same values as coded in 
obstacle_types = ("no_obstacle",
                  "start_of_bus_zone",
                  "end_of_bus_zone",
                  "start_of_yellow_box",
                  "end_of_yellow_box",
                  "road_bump",
                  "start_of_restrictive_bus_lane_section",
                  "end_of_restrictive_bus_lane_section",
                  "start_of_zebra_crossing",
                  "end_of_zebra_crossing",
                  "start_of_pedestrian_crossing",
                  "end_of_pedestrian_crossing",
                  "stop_line",
                  "ERP_gantry",
                  "turning_point")

class Road_item_with_obstacle_points:
    def __get_obstacle_points(self):
        if not hasattr(self, "obstacle_points__"):
            count = struct.unpack("B", self.database_obstacle_points)
            format_string = "B" * count
            obstacle_points__ = struct.unpack(format_string, self.database_obstacle_points[1:])
        return obstacle_points__
    def __set_obstacle_points(self, array):
        if len(array) > 256:
            raise RuntimeError("polyline with more than 256 obstacle-points is not supported")
        format_string = "B" * (1 + len(array))
        self.database_obstacle_points = struct.pack(format_string, len(array), *array)
    obstacle_points = property(__get_obstacle_points, __set_obstacle_points)

for i, name in enumerate(obstacle_types):
    python_code = "Road_item_with_obstacle_points.%s = %d" % (name, i)
    exec(python_code)

# Please ensure the following symbolic constants have the same values as coded in
# geospatial/Lane.hpp so that pqrne can output a string pattern of '0's and '1's
# that Lane.hpp will decode correctly.
# If you add more rules, please update both this file and Lane.hpp and in the same order.
traffic_flags = ("is_bicycle_lane",
                 "is_pedestrian_lane",
                 "is_vehicle_lane",
                 "is_high_occupany_vehicle_lane",
                 "can_vehicle_make_u_turn_here",
                 "can_vehicles_go_straight_ahead",
                 "can_vehicles_turn_left",
                 "can_vehicles_turn_right",
                 "can_vehicles_change_lane_left",
                 "can_vehicles_change_lane_right",
                 "can_vehicles_turn_on_red_signal",
                 "can_vehicles_stop_here",
                 "can_vehicles_freely_park_here",
                 "is_standard_bus_lane",
                 "is_whole_day_bus_lane",
                 "is_road_shoulder",
                 "max_traffic_rules")

class Lane(elixir.Entity, Road_item_with_polyline, Road_item_with_obstacle_points):
    """A lane represents a lane, in particularly the polyline field represents the lane's center-
    line; vehicles should be travelling along this polyline unless the driver intends to change
    lanes."""
    elixir.using_options(tablename="lanes")
    # "max_traffic_rules" is just a placeholder, it is not a valid rule.  The traffic_rules is
    # a string of '0' and '1', one for each of the rules above, not including "max_traffic_rules".
    traffic_rules = elixir.Field(elixir.Text, required=True,
                                 default='0' * (len(traffic_flags) - 1))
    database_polyline = elixir.Field(elixir.Binary)
    database_obstacle_points = elixir.Field(elixir.Binary)
    # A road segment has one or more lanes.  In each road-segment the lanes are numbered from right
    # to left when we are facing in the forward traffic flow.  The lane_index is the 0-based number.
    road_segment = elixir.ManyToOne("Road_segment")
    lane_index = elixir.Field(elixir.SmallInteger)
    width = elixir.Field(elixir.Float)
    lane_connectors = elixir.ManyToMany("Lane_connector")

class Lane_rule_getter_and_setter(object):
    # In python, indexing into strings are from left to right.  For example,
    #     s = "hello"
    # s[0] is 'h' and s[1] is 'e', etc.
    #
    # However, indexing into C++ std::bitset<> is from right to left; the most-signficant bit is
    # on the right.  For example,
    #     std::bitset<10> bitfields("100010111");
    # bitfields[0], bitfields[1], bitfields[2], and bitfields[4], and bitfields[8] is true;
    # the rest are false.
    #
    # Because python strings and std::bitset<> use different bit-ordering, we have convoluted
    # code in __get__ and __set__ below.

    def __init__(self, index):
        self.index = index

    def __get__(self, obj, type=None):
        # Return true if the text in the self.index position (counting from the right) is '1'
        return obj.traffic_rules[len(obj.traffic_rules) - 1 - self.index] == '1'

    def __set__(self, obj, value):
        rules = ''
        for i, text in enumerate(reversed(obj.traffic_rules)):
            if i != self.index:
                rules = text + rules
            else:
                rules = ('1' if value else '0') + rules
        obj.traffic_rules = rules

# Add properties to the Lane class for each name in the traffic_flags list.
for i, property_name in enumerate(traffic_flags):
    python_code = "Lane.%s = Lane_rule_getter_and_setter(%d)" % (property_name, i)
    exec(python_code)

# At this point, the Lane class has been endowed with properties matching the names in the
# traffic_flags list.  Thus,
#     lane = Lane()
#     lane.can_vehicles_turn_right = True
#     if lane.is_standard_bus_lane:
#         ....
# should not raise any exception.

class Road_segment(elixir.Entity):
    elixir.using_options(tablename="road_segments")
    lanes = elixir.OneToMany("Lane")
    link = elixir.ManyToOne("Link")
    segment_index = elixir.Field(elixir.SmallInteger)
    is_forward = elixir.Field(elixir.Boolean)

class Link(elixir.Entity):
    elixir.using_options(tablename="links")
    intersections = elixir.ManyToMany("Intersection")
    road_segments = elixir.OneToMany("Road_segment")
    road = elixir.ManyToOne("Road")

    def forward_road_segments(self):
        return [road_segment for road_segment in self.road_segment if road_segment.is_forward]

    def reverse_road_segments(self):
        return [road_segment for road_segment in self.road_segment if not road_segment.is_forward]

class Road(elixir.Entity):
    elixir.using_options(tablename="roads")
    links = elixir.OneToMany("Link") 
    name = elixir.Field(elixir.Text)

class Intersection(elixir.Entity):
    elixir.using_options(tablename="intersections")
    links = elixir.ManyToMany("Link")

class Lane_connector(elixir.Entity, Road_item_with_polyline, Road_item_with_obstacle_points):
    elixir.using_options(tablename="lane_connectors")
    lanes = elixir.ManyToMany("Lane")
    database_polyline = elixir.Field(elixir.Binary)
    obstacle_points = elixir.Field(elixir.Binary)
    turning_radius = elixir.Field(elixir.Float)

def convert_to_wkb_polyline(polyline):
    geometry = ogr.Geometry(ogr.wkbLineString)
    for i, point in enumerate(polyline):
        geometry.SetPoint(i, point.x, point.y, point.z)
    return geometry.ExportToWkb()

def convert_to_wkb_point(point):
    p = ogr.Geometry(ogr.wkbPoint)
    p.SetPoint(0, point.x, point.y, point.z)
    return p.ExportToWkb()

def convert_from_wkb_polyline(wkb):
    geometry = ogr.CreateGeometryFromWkb(wkb)
    polyline = list()
    for i in range(geometry.GetPointCount()):
        p = geometry.GetPoint(i)
        polyline.append(Point(p[0], p[1], p[2]))
    return polyline

def convert_from_wkb_point(wkb):
    p = ogr.CreateGeometryFromWkb(wkb)
    return Point(p.GetX(), p.GetY(), p.GetZ())

# vim:columns=100:smartindent:shiftwidth=4:expandtab:softtabstop=4:
