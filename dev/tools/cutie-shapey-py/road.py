#!/usr/bin/python

# Copyright Singapore-MIT Alliance for Research and Technology

import shapefile
from point import Point, Bounding_box
from PyQt4 import QtGui, QtCore

class Road:
    def __init__(self, id, carriage_ways_count, one_way, type, lanes_count, code, name, polyline):
        self.id = id
        self.carriage_ways_count = carriage_ways_count
        self.one_way = one_way
        self.type = type
        self.lanes_count = lanes_count
        self.code = code
        self.name = name.title()
        self.polyline = polyline

    def __repr__(self):
        return "road id=%d name='%s' polyline=%s" % (self.id, self.name, self.polyline)

    @staticmethod
    def column_names():
        return ("#", "Name", "cway-num", "one-way-in", "Type", "Lane count", "Shape-type", "Vertex")

    def columns(self):
        return (self.id, self.name, self.carriage_ways_count, self.one_way, self.type,
                self.lanes_count, "polyline", self.polyline)

    def tip_columns(self):
        return (0, 3)

    def tips(self):
        return (self.code, self.type_desc())

    def type_desc(self):
        if 'EP' == self.type: return "Expunged"
        if 'EX' == self.type: return "Expressway"
        if 'I' == self.type: return "Imaginary Line"
        if 'LA' == self.type: return "Local Access"
        if 'LCPA' == self.type: return "Local Collector/Primary Access"
        if 'MA' == self.type: return "Major Arterials/Minor Arterials"
        if 'N' == self.type: return "N"
        if 'OJ' == self.type: return "Other Junction"
        if 'PM' == self.type: return "Pedestrian Mall"
        if 'SR' == self.type: return "Slip Road"
        if 'TJ' == self.type: return "T-Junction"
        if 'TJ2' == self.type: return "2 T-Junction opposite each other"
        if 'U' == self.type: return "U"
        if 'XJ' == self.type: return "Cross Junction"
        if 'Y' == self.type: return "Y"

    def graphics(self):
        point = self.polyline[0]
        path = QtGui.QPainterPath(QtCore.QPointF(point.x, point.y))
        for point in self.polyline[1:]:
            path.lineTo(point.x, point.y)
        item = QtGui.QGraphicsPathItem(path)
        item.setPen(QtCore.Qt.red)
        item.info = "road id=%d name='%s' type='%s'" % (self.id, self.name, self.type_desc())
        return item

class Roads:
    def __init__(self, shape_name):
        sf = shapefile.Reader(shape_name)

        box = sf.bbox
        lower_left = Point(box[0], box[1])
        upper_right = Point(box[2], box[3])
        self.bounding_box = Bounding_box(lower_left, upper_right)

        self.roads = list()
        self.process_records_and_shapes(sf.numRecords, sf.records(), sf.shapes())

    @staticmethod
    def column_names():
        return Road.column_names()

    def error(self, message):
        print "error in road-attributes: %s" % message

    def process_records_and_shapes(self, count, records, shapes):
        for i in range(count):
            rec = records[i]

            if 7 != len(rec):
                self.error("records[%d] has %d fields" % (i, len(rec)))

            if not isinstance(rec[0], int):
                self.error("records[%d][0]='%s'" % (i, str(rec[0])))
            number_of_carriageways = rec[0]

            if rec[1] not in "YNU":
                self.error("records[%d][1]='%s'" % (i, rec[1]))
            one_way = rec[1]

            if rec[2] not in "INUY" and rec[2] not in ("EP", "EX", "LA", "LCPA", "MA", "OJ", "PM", "SR", "TJ", "TJ2", "XJ"):
                self.error("records[%d][2]='%s' records[%d][3]='%s'" % (i, rec[2], i, rec[3]))
                continue
            type = rec[2]

            if 'EP' == rec[2] and "Expunged" != rec[3]:
                self.error("records[%d][3]='%s'" % (i, rec[3]))
            if 'EX' == rec[2] and "Expressway" != rec[3]:
                self.error("records[%d][3]='%s'" % (i, rec[3]))
            if 'I' == rec[2] and "Imaginary Line" != rec[3]:
                self.error("records[%d][3]='%s'" % (i, rec[3]))
            if 'LA' == rec[2] and "Local Access" != rec[3]:
                self.error("records[%d][3]='%s'" % (i, rec[3]))
            if 'LCPA' == rec[2] and "Local Collector/Primary Access" != rec[3]:
                self.error("records[%d][3]='%s'" % (i, rec[3]))
            if 'MA' == rec[2] and "Major Arterials/Minor Arterials" != rec[3]:
                self.error("records[%d][3]='%s'" % (i, rec[3]))
            if 'N' == rec[2] and "N" != rec[3]:
                self.error("records[%d][3]='%s'" % (i, rec[3]))
            if 'OJ' == rec[2] and "Other Junction" != rec[3]:
                self.error("records[%d][3]='%s'" % (i, rec[3]))
            if 'PM' == rec[2] and "Pedestrian Mall" != rec[3]:
                self.error("records[%d][3]='%s'" % (i, rec[3]))
            if 'SR' == rec[2] and "Slip Road" != rec[3]:
                self.error("records[%d][3]='%s'" % (i, rec[3]))
            if 'TJ' == rec[2] and "T-Junction" != rec[3]:
                self.error("records[%d][3]='%s'" % (i, rec[3]))
            if 'TJ2' == rec[2] and "2 T-Junction opposite each other" != rec[3]:
                self.error("records[%d][3]='%s'" % (i, rec[3]))
            if 'U' == rec[2] and "U" != rec[3]:
                self.error("records[%d][3]='%s'" % (i, rec[3]))
            if 'XJ' == rec[2] and "Cross Junction" != rec[3]:
                self.error("records[%d][3]='%s'" % (i, rec[3]))
            if 'Y' == rec[2] and "Y" != rec[3]:
                self.error("records[%d][3]='%s'" % (i, rec[3]))

            if not isinstance(rec[4], int):
                self.error("records[%d][4]='%s'" % (i, str(rec[4])))
            number_of_lanes = rec[4]

            if not isinstance(rec[5], str):
                self.error("records[%d][5]='%s'" % (i, str(rec[5])))
            code = rec[5]

            if not isinstance(rec[6], str):
                self.error("records[%d][6]='%s'" % (i, str(rec[6])))
            name = rec[6]

            shape = shapes[i]
            if 3 != shape.shapeType:
                self.error("shapes[%d].shapeType=%d" % (i, shape.shapeType))
            if 1 != len(shape.parts) and 0 != self.parts[0]:
                self.error("shapes[%d].parts=%s" % (i, shape.parts))
            polyline = list()
            for point in shape.points:
                polyline.append(Point(point[0], point[1]))

            self.roads.append(Road(i, number_of_carriageways, one_way, type,
                                   number_of_lanes, code, name, polyline))

    def records(self):
        return self.roads

if "__main__" == __name__:
    def error():
        print "Usage: %s data-folder" % sys.argv[0]
        print "must specify the folder containing the road-attributes shape files"
        sys.exit(1)

    import sys
    if len(sys.argv) != 2:
        error()

    try:
        roads = Roads("%s/RoadAttributeLine_15Sep10_104252" % sys.argv[1])
    except shapefile.ShapefileException as reason:
        print reason
        error()

    print "bounding box =", roads.bounding_box
    for road in roads.roads:
        print road
