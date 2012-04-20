#!/usr/bin/env python

# Copyright (2012) Singapore-MIT Alliance for Research and Technology

import elixir
import sqlalchemy
from osgeo import ogr

from point import Point

__metadata__ = sqlalchemy.schema.ThreadLocalMetaData()
__session__ = sqlalchemy.orm.scoped_session(sqlalchemy.orm.sessionmaker())

class Digital_map:
    road_types = dict()
    lane_marking_types = dict()
    arrow_marking_types = dict()
    traffic_signal_types = dict()

    def __init__(self, database_name):
        self.roads = dict()
        self.lane_markings = dict()
        self.kerb_lines = list()
        self.arrow_markings = list()
        self.bus_stops = list()
        self.traffic_signals = list()

        self.road_types.clear()
        self.lane_marking_types.clear()
        self.arrow_marking_types.clear()
        self.traffic_signal_types.clear()

        self.engine = sqlalchemy.create_engine(database_name)
        __session__.configure(autoflush=True, bind=self.engine)
        __metadata__.bind = self.engine

    def is_dirty(self):
        return __session__.new or __session__.dirty or __session__.deleted

    def commit_to_database(self):
        __session__.commit()

    def add_road_section(self, road_name, road_type, type_description,
                         cway_number, one_way_in, lane_count, polyline):
        road_section = Road_section(road_name=road_name, road_type=road_type,
                                    cway_number=cway_number, one_way_in=one_way_in,
                                    lane_count=lane_count)
        road_section.polyline = polyline
        if road_name not in self.roads:
            self.roads[road_name] = list()
        self.roads[road_name].append(road_section)
        if road_type not in self.road_types:
            self.road_types[road_type] = Road_type(type_code=road_type,
                                                   type_description=type_description)

    def add_lane_marking(self, marking_type, type_description, polyline):
        lane_marking = Lane_marking(marking_type=marking_type)
        lane_marking.polyline = polyline
        if marking_type not in self.lane_markings:
            self.lane_markings[marking_type] = list()
        self.lane_markings[marking_type].append(lane_marking)
        if marking_type not in self.lane_marking_types:
            lane_marking_type = Lane_marking_type(type_code=marking_type,
                                                  type_description=type_description)
            self.lane_marking_types[marking_type] = lane_marking_type
        return lane_marking

    def add_kerb_line(self, polyline):
        kerb_line = Kerb_line()
        kerb_line.polyline = polyline
        self.kerb_lines.append(kerb_line)

    def add_arrow_marking(self, bearing, arrow_type, type_description, position):
        arrow_marking = Arrow_marking(bearing=bearing, arrow_type=arrow_type)
        arrow_marking.position = position
        self.arrow_markings.append(arrow_marking)
        if arrow_type not in self.arrow_marking_types:
            arrow_marking_type = Arrow_marking_type(type_code=arrow_type,
                                                    type_description=type_description)
            self.arrow_marking_types[arrow_type] = arrow_marking_type

    def add_bus_stop(self, bus_stop_number, status, position):
        bus_stop = Bus_stop(bus_stop_number=bus_stop_number, status=status)
        bus_stop.position = position
        self.bus_stops.append(bus_stop)

    def add_traffic_signal(self, bearing, signal_type, type_description, position):
        traffic_signal = Traffic_signal(bearing=bearing, signal_type=signal_type)
        traffic_signal.position = position
        self.traffic_signals.append(traffic_signal)
        if signal_type not in self.traffic_signal_types:
            traffic_signal_type = Traffic_signal_type(type_code=signal_type,
                                                      type_description=type_description)
            self.traffic_signal_types[signal_type] = traffic_signal_type

    def load_database(self):
        for section in Road_section.query.all():
            if section.road_name not in self.roads:
                self.roads[section.road_name] = list()
            self.roads[section.road_name].append(section)
        for road_type in Road_type.query.all():
            self.road_types[road_type.type_code] = road_type.type_description

        for lane_marking in Lane_marking.query.all():
            if lane_marking.marking_type not in self.lane_markings:
                self.lane_markings[lane_marking.marking_type] = list()
            self.lane_markings[lane_marking.marking_type].append(lane_marking)
        for marking_type in Lane_marking_type.query.all():
            self.lane_marking_types[marking_type.type_code] = marking_type.type_description

        for kerb_line in Kerb_line.query.all():
            self.kerb_lines.append(kerb_line)

        for arrow_marking in Arrow_marking.query.all():
            self.arrow_markings.append(arrow_marking)
        for marking_type in Arrow_marking_type.query.all():
            self.arrow_marking_types[marking_type.type_code] = marking_type.type_description

        for bus_stop in Bus_stop.query.all():
            self.bus_stops.append(bus_stop)

        for traffic_signal in Traffic_signal.query.all():
            self.traffic_signals.append(traffic_signal)
        for signal_type in Traffic_signal_type.query.all():
            self.traffic_signal_types[signal_type.type_code] = signal_type.type_description

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

class Road_section(elixir.Entity, Road_item_with_polyline):
    elixir.using_options(tablename="road_sections")
    road_name = elixir.Field(elixir.Text)
    road_type = elixir.Field(elixir.Text)
    cway_number = elixir.Field(elixir.SmallInteger)
    one_way_in = elixir.Field(elixir.Text)
    lane_count = elixir.Field(elixir.SmallInteger)
    database_polyline = elixir.Field(elixir.Binary)

    def __repr__(self):
        return "road name='%s' type='%s'" % (self.road_name, self.road_type)

    def road_info(self):
        info =   "road-section={\n" \
               + "    road-name='%s'\n" % self.road_name \
               + "    road-type='%s' (%s)\n" % (self.road_type,
                                                Digital_map.road_types[self.road_type]) \
               + "    carriage-way-number=%d\n" % self.cway_number \
               + "    one-way-in='%s'\n" % self.one_way_in \
               + "    lane-count='%d'\n" % self.lane_count \
               + "    polyline={\n"
        for i, point in enumerate(self.polyline):
            info = info + "        %4d   %s\n" % (i, point)
        info = info + "    }\n}\n"
        return info

class Road_type(elixir.Entity):
    elixir.using_options(tablename="road_types")
    type_code = elixir.Field(elixir.Text, primary_key=True)
    type_description = elixir.Field(elixir.Text)

class Lane_marking(elixir.Entity, Road_item_with_polyline):
    elixir.using_options(tablename="lane_markings")
    marking_type = elixir.Field(elixir.Text)
    database_polyline = elixir.Field(elixir.Binary)

    def __repr__(self):
        return "lane-marking type='%s'" % (self.marking_type)

    def road_info(self):
        marking_type = self.marking_type
        info =   "lane-marking={\n" \
               + "    marking-type='%s' (%s)\n" % (marking_type,
                                                   Digital_map.lane_marking_types[marking_type]) \
               + "    polyline={\n"
        for i, point in enumerate(self.polyline):
            info = info + "        %4d   %s\n" % (i, point)
        info = info + "    }\n}\n"
        return info

class Lane_marking_type(elixir.Entity):
    elixir.using_options(tablename="lane_marking_types")
    type_code = elixir.Field(elixir.Text, primary_key=True)
    type_description = elixir.Field(elixir.Text)

class Kerb_line(elixir.Entity, Road_item_with_polyline):
    elixir.using_options(tablename="kerb_lines")
    database_polyline = elixir.Field(elixir.Binary)

    def __repr__(self):
        return "kerb-line"

    def road_info(self):
        info =   "kerb-line={\n" \
               + "    polyline={\n"
        for i, point in enumerate(self.polyline):
            info = info + "        %4d   %s\n" % (i, point)
        info = info + "    }\n}\n"
        return info

class Arrow_marking(elixir.Entity, Road_item_at_a_point):
    elixir.using_options(tablename="arrow_markings")
    arrow_type = elixir.Field(elixir.Text)
    bearing = elixir.Field(elixir.Float)
    database_position = elixir.Field(elixir.Binary)

    def __repr__(self):
        return "arrow-marking"

    def road_info(self):
        arrow_type = self.arrow_type
        info =   "arrow-marking={\n" \
               + "    arrow-type='%s' (%s)\n" % (arrow_type,
                                                 Digital_map.arrow_marking_types[arrow_type]) \
               + "    bearing=%f\n" % self.bearing \
               + "    position=%s\n" % self.position \
               + "}\n"
        return info

class Arrow_marking_type(elixir.Entity):
    elixir.using_options(tablename="arrow_marking_types")
    type_code = elixir.Field(elixir.Text, primary_key=True)
    type_description = elixir.Field(elixir.Text)

class Bus_stop(elixir.Entity, Road_item_at_a_point):
    elixir.using_options(tablename="bus_stops")
    bus_stop_number = elixir.Field(elixir.Text)
    status = elixir.Field(elixir.Text)
    database_position = elixir.Field(elixir.Binary)

    def __repr__(self):
        return "bus-stop"

    def road_info(self):
        info =   "bus-stop={\n" \
               + "    bus-stop-number='%s'\n" % self.bus_stop_number \
               + "    status=%s" % "operational" if self.status == "OP" else "non-operational" \
               + "    position=%s" % self.position \
               + "}\n"
        return info

class Traffic_signal(elixir.Entity, Road_item_at_a_point):
    elixir.using_options(tablename="traffic_signals")
    signal_type = elixir.Field(elixir.Text)
    bearing = elixir.Field(elixir.Float)
    database_position = elixir.Field(elixir.Binary)

    def __repr__(self):
        return "traffic-signal type='%s'" % (self.signal_type)

    def road_info(self):
        signal_type = self.signal_type
        info =   "traffic-signal={\n" \
               + "    signal-type='%s' (%s)\n" % (signal_type,
                                                  Digital_map.traffic_signal_types[signal_type]) \
               + "    bearing=%f\n" % self.bearing \
               + "    position=%s\n" % self.position \
               + "}\n"
        return info

class Traffic_signal_type(elixir.Entity):
    elixir.using_options(tablename="traffic_signal_types")
    type_code = elixir.Field(elixir.Text, primary_key=True)
    type_description = elixir.Field(elixir.Text)

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
