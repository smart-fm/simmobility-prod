#!/usr/bin/python

# Copyright Singapore-MIT Alliance for Research and Technology

import shapefile
from point import Point, Bounding_box
from PyQt4 import QtGui, QtCore

class Bus_stop:
    def __init__(self, id, position, number):
        self.id = id
        self.position = position
        self.number = number

    def __repr__(self):
        return "Bus-stop id=%d position=%s number=%d" % (self.id, self.position, self.number)

    @staticmethod
    def column_names():
        return ("Bus-stop number", "Shape type", "Position")

    def columns(self):
        return (self.number, "point", self.position)

    def tip_columns(self):
        return list()

    def tips(self):
        return list()

    def graphics(self):
        path = QtGui.QPainterPath(QtCore.QPointF(-15, 0))
        path.lineTo(15, 0)
        path.lineTo(15, 240)
        path.lineTo(135, 240)
        path.lineTo(135, 330)
        path.lineTo(-15, 330)
        path.closeSubpath()
        item = QtGui.QGraphicsPathItem(path)
        #item.setPen(QtCore.Qt.green)
        item.setBrush(QtCore.Qt.green)
        item.setPos(self.position.x, self.position.y)
        return item

class Bus_stops:
    def __init__(self, shape_name):
        sf = shapefile.Reader(shape_name)

        box = sf.bbox
        lower_left = Point(box[0], box[1])
        upper_right = Point(box[2], box[3])
        self.bounding_box = Bounding_box(lower_left, upper_right)

        self.bus_stops = list()
        self.process_records_and_shapes(sf.numRecords, sf.records(), sf.shapes())

    @staticmethod
    def column_names():
        return Bus_stop.column_names()

    def error(self, message):
        print "error in bus-stop: %s" % message

    def process_records_and_shapes(self, count, records, shapes):
        for i in range(count):
            rec = records[i]

            if 3 != len(rec):
                self.error("records[%d] has %d fields" % (i, len(rec)))

            # The first field is BUS_STOP_N, a string of 65 characters, although it seems that
            # this field in all records contains only characters in "0123456789".
            if not isinstance(rec[0], str):
                self.error("records[%d][0]='%s' (not a string)" % (i, str(rec[0])))
            try:
                stop_number = int(rec[0])
            except ValueError:
                self.error("records[%d][0]='%s'" % (i, rec[0]))
                continue

            if "OP" == rec[1]:
                status = rec[1]
                if "OPERATIONAL" != rec[2]:
                    self.error("records[%d][2]='%s'" % (i, rec[2]))
            elif "NOP" == rec[1]:
                status = rec[1]
                if "NON-OPERATIONAL" != rec[2]:
                    self.error("records[%d][2]='%s'" % (i, rec[2]))
            else:
                self.error("records[%d][1]='%s'" % (i, rec[1]))

            shape = shapes[i]
            if 1 != shape.shapeType:
                self.error("shapes[%d].shapeType=%d" % (i, shape.shapeType))
            if 1 != len(shape.points):
                self.error("shapes[%d].points has %d items" % (i, len(shape.points)))

            point = shape.points[0]
            position = Point(point[0], point[1])

            self.bus_stops.append(Bus_stop(i, position, stop_number))

    def records(self):
        return self.bus_stops

if "__main__" == __name__:
    def error():
        print "Usage: %s data-folder" % sys.argv[0]
        print "must specify the folder containing the bus-stops shape files"
        sys.exit(1)

    import sys
    if len(sys.argv) != 2:
        error()

    try:
        bus_stops = Bus_stops("%s/BusStop_15Sep10_104241" % sys.argv[1])
    except shapefile.ShapefileException as reason:
        print reason
        error()

    print "bounding box =", bus_stops.bounding_box
    for bus_stop in bus_stops.bus_stops:
        print bus_stop
