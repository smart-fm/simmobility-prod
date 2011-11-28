#!/usr/bin/python

# Copyright Singapore-MIT Alliance for Research and Technology

import shapefile
from point import Point, Bounding_box
from PyQt4 import QtGui, QtCore

class Kerb_line:
    def __init__(self, id, polyline):
        self.id = id
        self.polyline = polyline

    def __repr__(self):
        return "kerb-line id=%d polyline=%s" % (self.id, self.polyline)

    @staticmethod
    def column_names():
        return ("#", "Shape-type", "Vertex")

    def columns(self):
        return (self.id, "polyline", self.polyline)

    def tip_columns(self):
        return list()

    def tips(self):
        return list()

    def graphics(self):
        point = self.polyline[0]
        path = QtGui.QPainterPath(QtCore.QPointF(point.x, point.y))
        for point in self.polyline[1:]:
            path.lineTo(point.x, point.y)
        item = QtGui.QGraphicsPathItem(path)
        item.setPen(QtCore.Qt.black)
        item.info = "kerb line id=%d" % (self.id)
        return item

class Kerb_lines:
    def __init__(self, shape_name):
        sf = shapefile.Reader(shape_name)

        box = sf.bbox
        lower_left = Point(box[0], box[1])
        upper_right = Point(box[2], box[3])
        self.bounding_box = Bounding_box(lower_left, upper_right)

        self.kerb_lines = list()
        self.process_records_and_shapes(sf.numRecords, sf.records(), sf.shapes())

    @staticmethod
    def column_names():
        return Kerb_line.column_names()

    def error(self, message):
        print "error in kerb-lines: %s" % message

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

            if 1 != len(rec):
                self.error("records[%d] has %d fields" % (i, len(rec)))
            if not self.is_all_blanks(rec[0]):
                self.error("records[%d][0]='%s'" % (i, str(rec[0])))

            shape = shapes[i]
            if 3 != shape.shapeType:
                self.error("shapes[%d].shapeType=%d" % (i, shape.shapeType))
            if 1 != len(shape.parts) and 0 != self.parts[0]:
                self.error("shapes[%d].parts=%s" % (i, shape.parts))
            polyline = list()
            for point in shape.points:
                polyline.append(Point(point[0], point[1]))

            self.kerb_lines.append(Kerb_line(i, polyline))

    def records(self):
        return self.kerb_lines

if "__main__" == __name__:
    def error():
        print "Usage: %s data-folder" % sys.argv[0]
        print "must specify the folder containing the kerb-lines shape files"
        sys.exit(1)

    import sys
    if len(sys.argv) != 2:
        error()

    try:
        kerb_lines = Kerb_lines("%s/KerbLine_15Sep10_104924" % sys.argv[1])
    except shapefile.ShapefileException as reason:
        print reason
        error()

    print "bounding box =", kerb_lines.bounding_box
    for kerb_line in kerb_lines.kerb_lines:
        print kerb_line
