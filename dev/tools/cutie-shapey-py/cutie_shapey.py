#!/usr/bin/python

import sys
import shapefile
import road
import arrow
import kerb
import lane
import stop
import tsignal
import window
from PyQt4 import QtGui, QtCore

def error():
    print "Usage: %s data-folder" % sys.argv[0]
    print "must specify the folder containing the various shape files"
    sys.exit(1)

def cutie_shapey_py(road_attributes_shape_name,
                    arrow_markings_shape_name,
                    kerb_lines_shape_name,
                    lane_markings_shape_name,
                    bus_stops_shape_name,
                    traffic_signals_shape_name):
    if len(sys.argv) != 2:
        error()

    try:
        road_attributes = road.Roads("%s/%s" % (sys.argv[1], road_attributes_shape_name))
        arrow_markings = arrow.Arrow_markings("%s/%s" % (sys.argv[1], arrow_markings_shape_name))
        kerb_lines = kerb.Kerb_lines("%s/%s" % (sys.argv[1], kerb_lines_shape_name))
        lane_markings = lane.Lane_markings("%s/%s" % (sys.argv[1], lane_markings_shape_name))
        bus_stops = stop.Bus_stops("%s/%s" % (sys.argv[1], bus_stops_shape_name))
        traffic_signals = tsignal.Traffic_signals("%s/%s" % (sys.argv[1], traffic_signals_shape_name))
    except shapefile.ShapefileException as reason:
        print reason
        error()

    app = QtGui.QApplication(sys.argv)
    win = window.Main_window()
    win.add_shape_file("Road attributes", road_attributes)
    win.add_shape_file("Arrow markings", arrow_markings)
    win.add_shape_file("Kerb lines", kerb_lines)
    win.add_shape_file("Lane markings", lane_markings)
    win.add_shape_file("Bus stops", bus_stops)
    win.add_shape_file("Traffic signals", traffic_signals)
    win.show()

    sys.exit(app.exec_())

if "__main__" == __name__:
    road_attributes_shape_name = "RoadAttributeLine_15Sep10_104252"
    arrow_markings_shape_name = "ArrowMarking_15Sep10_104237"
    kerb_lines_shape_name = "KerbLine_15Sep10_104924"
    lane_markings_shape_name = "LaneMarking_15Sep10_104241"
    bus_stops_shape_name = "BusStop_15Sep10_104241"
    traffic_signals_shape_name = "TrafficSignalAspect_15Sep10_104655"
    cutie_shapey_py(road_attributes_shape_name,
                    arrow_markings_shape_name,
                    kerb_lines_shape_name,
                    lane_markings_shape_name,
                    bus_stops_shape_name,
                    traffic_signals_shape_name)
