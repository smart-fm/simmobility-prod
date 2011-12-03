#!/usr/bin/python

# Copyright Singapore-MIT Alliance for Research and Technology

# Couldn't name this module as "signal.py" because "signal" is used in somewhere in Qt.

import shapefile
from point import Point, Bounding_box
from PyQt4 import QtGui, QtCore

class Traffic_signal:
    def __init__(self, id, position, bearing, type):
        self.id = id
        self.position = position
        self.bearing = bearing
        self.type = type

    def __repr__(self):
        return "Traffic-signal id=%d position=%s bearing=%d type=%s" % (self.id, self.position,
                                                                       self.bearing, self.type)

    @staticmethod
    def column_names():
        return ("#", "Type", "Bearing", "Shape type", "Position")

    def columns(self):
        return (self.id, self.type, self.bearing, "point", self.position)

    def tip_columns(self):
        return [0]

    def tips(self):
        return [self.type_desc()]

    def type_desc(self):
        if 'B' == self.type: return "B-Signal"
        if 'C' == self.type: return "Ovelhead Signal Centre Median"
        if 'F' == self.type: return "Green Filter Arrow Signal"
        if 'G' == self.type: return "Ground Signal"
        if 'H' == self.type: return "Overhead Signal"
        if 'N' == self.type: return "Beacon"
        if 'P' == self.type: return "Pedestrian Signal"
        if 'R' == self.type: return "RAG"
        if 'T' == self.type: return "Count Down Timer for Pedestrian"

    def graphics(self):
        # The signal icon looks like an ice-cream cone, with the semi-circle representing the
        # light bulb facing the drivers or pedestrians.
        path1 = QtGui.QPainterPath()
        path1.lineTo(-50, 200)
        path1.lineTo(50, 200)
        path1.closeSubpath()
        path2 = QtGui.QPainterPath(QtCore.QPointF(-50, 200))
        path2.arcTo(-50, 150, 100, 100, 0, -180)
        path2.lineTo(0, 0)
        path2.closeSubpath()
        item = QtGui.QGraphicsPathItem(path1.united(path2))

        if 'B' == self.type: item.setBrush(QtCore.Qt.red)
        if 'C' == self.type: item.setBrush(QtCore.Qt.blue)
        if 'F' == self.type: item.setBrush(QtCore.Qt.green)
        if 'G' == self.type: item.setBrush(QtCore.Qt.cyan)
        if 'H' == self.type: item.setBrush(QtCore.Qt.magenta)
        if 'N' == self.type: item.setBrush(QtCore.Qt.yellow)
        if 'P' == self.type: item.setBrush(QtCore.Qt.black)
        if 'R' == self.type: item.setBrush(QtCore.Qt.gray)
        if 'T' == self.type: item.setBrush(QtCore.Qt.lightGray)
        item.setPos(self.position.x, self.position.y)
        # See the comments in the Graphics_view constructor in window.py
        item.setRotation(-self.bearing)
        item.info = "signal id=%d type='%s' bearing=%d" % (self.id, self.type_desc(), self.bearing)
        return item

class Traffic_signals:
    def __init__(self, shape_name):
        sf = shapefile.Reader(shape_name)

        box = sf.bbox
        lower_left = Point(box[0], box[1])
        upper_right = Point(box[2], box[3])
        self.bounding_box = Bounding_box(lower_left, upper_right)

        self.traffic_signals = list()
        self.process_records_and_shapes(sf.numRecords, sf.records(), sf.shapes())

    @staticmethod
    def column_names():
        return Traffic_signal.column_names()

    def error(self, message):
        print "error in traffic-signals: %s" % message

    def is_all_blanks(self, field):
        if not isinstance(field, str):
            return False
        for c in field:
            if ' ' != c:
                return False
        return True

    def process_records_and_shapes(self, count, records, shapes):
        for i in range(count):
            rec = records[i]

            if 4 != len(rec):
                self.error("records[%d] has %d fields" % (i, len(rec)))
            if not isinstance(rec[0], float):
                self.error("records[%d][0]='%s'" % (i, str(rec[0])))
            bearing = int(rec[0])

            if not self.is_all_blanks(rec[1]):
                self.error("records[%d][1]='%s'" % (i, rec[1]))

            if rec[2] not in "BCFGHNPRT":
                self.error("records[%d][2]='%s' records[%d][3]='%s' shapes[%d].shapeType=%d shapes[%d].points=%s" % (i, rec[2], i, rec[3], i, shapes[i].shapeType, i, shapes[i].points))
                continue
            type = rec[2]

            if 'B' == rec[2] and "B-Signal" != rec[3]:
                self.error("records[%d][3]='%s'" % (i, rec[3]))
            if 'C' == rec[2] and "Ovelhead Signal Centre Median" != rec[3]:
                self.error("records[%d][3]='%s'" % (i, rec[3]))
            if 'F' == rec[2] and "Green Filter Arrow Signal" != rec[3]:
                self.error("records[%d][3]='%s'" % (i, rec[3]))
            if 'G' == rec[2] and "Ground Signal" != rec[3]:
                self.error("records[%d][3]='%s'" % (i, rec[3]))
            if 'H' == rec[2] and "Overhead Signal" != rec[3]:
                self.error("records[%d][3]='%s'" % (i, rec[3]))
            if 'N' == rec[2] and "Beacon" != rec[3]:
                self.error("records[%d][3]='%s'" % (i, rec[3]))
            if 'P' == rec[2] and "Pedestrian Signal" != rec[3]:
                self.error("records[%d][3]='%s'" % (i, rec[3]))
            if 'R' == rec[2] and "RAG" != rec[3]:
                self.error("records[%d][3]='%s'" % (i, rec[3]))
            if 'T' == rec[2] and "Count Down Timer for Pedestrian" != rec[3]:
                self.error("records[%d][3]='%s'" % (i, rec[3]))

            shape = shapes[i]
            if 1 != shape.shapeType:
                self.error("shapes[%d].shapeType=%d" % (i, shape.shapeType))
            if 1 != len(shape.points):
                self.error("shapes[%d].points has %d items" % (i, len(shape.points)))

            point = shape.points[0]
            position = Point(point[0], point[1])

            self.traffic_signals.append(Traffic_signal(i, position, bearing, type))

    def records(self):
        return self.traffic_signals

if "__main__" == __name__:
    def error():
        print "Usage: %s data-folder" % sys.argv[0]
        print "must specify the folder containing the traffic-signals shape files"
        sys.exit(1)

    import sys
    if len(sys.argv) != 2:
        error()

    try:
        signals = Traffic_signals("%s/TrafficSignalAspect_15Sep10_104655" % sys.argv[1])
    except shapefile.ShapefileException as reason:
        print reason
        error()

    print "bounding box =", signals.bounding_box
    for signal in signals.traffic_signals:
        print signal
