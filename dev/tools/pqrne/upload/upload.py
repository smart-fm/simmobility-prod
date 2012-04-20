#!/usr/bin/env python

# Copyright (2012) Singapore-MIT Alliance for Research and Technology

import sys
import elixir
import shapefile

sys.path.insert(0, "../")
from digital_map import Digital_map
from point import Point

class Uploader:
    def __init__(self, database_name):
        self.digital_map = Digital_map(database_name)
        elixir.setup_all()
        elixir.create_all()

    def upload(self, shape_files):
        if len(sys.argv) != 2:
            reason = "usage: %s <top-dir>" % sys.argv[0]
            reason = reason + ", where <top-dir> is the directory containing the shape-files."
            raise RuntimeError(reason)
        top_dir = sys.argv[1]

        self.upload_road_attributes("%s/%s" % (top_dir, shape_files["road-attributes"]))
        self.upload_lane_markings("%s/%s" % (top_dir, shape_files["lane-markings"]))
        self.upload_kerb_lines("%s/%s" % (top_dir, shape_files["kerb-lines"]))
        self.upload_arrow_markings("%s/%s" % (top_dir, shape_files["arrow-markings"]))
        self.upload_bus_stops("%s/%s" % (top_dir, shape_files["bus-stops"]))
        self.upload_traffic_signals("%s/%s" % (top_dir, shape_files["traffic-signals"]))

        self.digital_map.commit_to_database()

    def upload_road_attributes(self, shape_file):
        sf = shapefile.Reader(shape_file)
        for i in range(sf.numRecords):
            rec = sf.record(i)
            cway_number = rec[0]
            one_way_in = rec[1]
            road_type = rec[2]
            type_description = rec[3]
            lane_count = rec[4]
            road_name = rec[6].title()

            shape = sf.shape(i)
            polyline = list()
            for point in shape.points:
                polyline.append(Point(point[0], point[1]))

            self.digital_map.add_road_section(road_name, road_type, type_description,
                                               cway_number, one_way_in, lane_count, polyline)

    def upload_lane_markings(self, shape_file):
        sf = shapefile.Reader(shape_file)
        for i in range(sf.numRecords):
            rec = sf.record(i)
            marking_type=rec[0]
            type_description=rec[1]

            shape = sf.shape(i)
            polyline = list()
            for point in shape.points:
                polyline.append(Point(point[0], point[1]))

            self.digital_map.add_lane_marking(marking_type, type_description, polyline)

    def upload_kerb_lines(self, shape_file):
        sf = shapefile.Reader(shape_file)
        for i in range(sf.numRecords):
            shape = sf.shape(i)
            polyline = list()
            for point in shape.points:
                polyline.append(Point(point[0], point[1]))

            self.digital_map.add_kerb_line(polyline)

    def upload_arrow_markings(self, shape_file):
        sf = shapefile.Reader(shape_file)
        for i in range(sf.numRecords):
            rec = sf.record(i)
            bearing=rec[0]
            arrow_type=rec[2]
            type_description=rec[3]

            shape = sf.shape(i)
            point = shape.points[0]
            position = Point(point[0], point[1])

            self.digital_map.add_arrow_marking(bearing, arrow_type, type_description, position)

    def upload_bus_stops(self, shape_file):
        sf = shapefile.Reader(shape_file)
        for i in range(sf.numRecords):
            rec = sf.record(i)
            bus_stop_number=rec[0]
            status=rec[1]

            shape = sf.shape(i)
            point = shape.points[0]
            position = Point(point[0], point[1])

            self.digital_map.add_bus_stop(bus_stop_number, status, position)

    def upload_traffic_signals(self, shape_file):
        sf = shapefile.Reader(shape_file)
        for i in range(sf.numRecords):
            rec = sf.record(i)
            bearing=rec[0]
            signal_type=rec[2]
            type_description=rec[3]

            shape = sf.shape(i)
            point = shape.points[0]
            position = Point(point[0], point[1])

            self.digital_map.add_traffic_signal(bearing, signal_type, type_description, position)

# vim:columns=100:smartindent:shiftwidth=4:expandtab:softtabstop=4:
