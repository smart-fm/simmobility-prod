#!/usr/bin/python

# Copyright Singapore-MIT Alliance for Research and Technology

import shapefile
from point import Point, Bounding_box
from PyQt4 import QtGui, QtCore

class Lane_marking:
    def __init__(self, id, type, polyline):
        self.id = id
        self.type = type
        self.polyline = polyline

    def __repr__(self):
        return "lane-markings id=%d type=%s polyline=%s" % (self.id, self.type, self.polyline)

    @staticmethod
    def column_names():
        return ("#", "Type", "Shape-type", "Vertex")

    def columns(self):
        return (self.id, self.type, "polyline", self.polyline)

    def tip_columns(self):
        return [0]

    def tips(self):
        return [self.type_desc()]

    def type_desc(self):
        if 'A' == self.type: return "1m,int 1m,0.1m white"
        if 'A1' == self.type: return "1m,int 1m,0.1m Yellow"
        if 'A2' == self.type: return "1m,int 1m,0.2m White"
        if 'A3' == self.type: return "1m,int 1m,0.3m yellow"
        if 'A4' == self.type: return "broken white lines at signalised pedestrian crossing"
        if 'B' == self.type: return "2m,int 4m,0.1m white"
        if 'B1' == self.type: return "2m,int 10m ,0.1m White"
        if 'C' == self.type: return "4m,int 2m,0.1m white"
        if 'D' == self.type: return "1m,int 1m,0.1m dbl white"
        if 'E' == self.type: return "2.75m,int 2.75m,0.15m white"
        if 'F' == self.type: return "Centre line,0.15 m white"
        if 'I' == self.type: return "continuous Double Yellow Line"
        if 'G' == self.type: return "Side Line,0.15 yellow"
        if 'H' == self.type: return "continuous Double White Line"
        if 'J' == self.type: return "White Line-Ped Crossing"
        if 'K' == self.type: return "Zig Zag Line"
        if 'L' == self.type: return "Yellow Line - bus lane"
        if 'M' == self.type: return "White Line-stop line"
        if 'N' == self.type: return "Yellow Box Line"
        if 'O' == self.type: return "Single yellow zig-zag"
        if 'P' == self.type: return "Double yellow zig-zag"
        if 'Q' == self.type: return "Multi-arrow"
        if 'R' == self.type: return "Bus Zone"
        if 'S' == self.type: return "continuous red line for full day bus lane"
        if 'S1' == self.type: return "Dotted red line for full day bus lane"
        if 'T' == self.type: return "Turning Pocket"
        if 'U' == self.type: return "Pedestrian crossing ahead marking"

    def graphics(self):
        point = self.polyline[0]
        path = QtGui.QPainterPath(QtCore.QPointF(point.x, point.y))
        for point in self.polyline[1:]:
            path.lineTo(point.x, point.y)
        item = QtGui.QGraphicsPathItem(path)
        item.setPen(QtCore.Qt.blue)
        item.info = "lane marking id=%d type='%s - %s'" % (self.id, self.type, self.type_desc())
        return item

class Lane_markings:
    def __init__(self, shape_name):
        sf = shapefile.Reader(shape_name)

        box = sf.bbox
        lower_left = Point(box[0], box[1])
        upper_right = Point(box[2], box[3])
        self.bounding_box = Bounding_box(lower_left, upper_right)

        self.lane_markings = list()
        self.process_records_and_shapes(sf.numRecords, sf.records(), sf.shapes())

    @staticmethod
    def column_names():
        return Lane_marking.column_names()

    def error(self, message):
        print "error in lane-markings: %s" % message

    def process_records_and_shapes(self, count, records, shapes):
        for i in range(count):
            rec = records[i]

            if 2 != len(rec):
                self.error("records[%d] has %d fields" % (i, len(rec)))
            if rec[0] not in "ABCDEFGHIJKLMNOPQRSTU" and rec[0] not in ("A1", "A2", "A3", "A4", "B1", "S1"):
                self.error("records[%d][0]='%s' records[%d][1]='%s'" % (i, rec[0], i, rec[1]))
                continue
            type = rec[0]

            if 'A' == rec[0] and "A -1m,int 1m,0.1m white" != rec[1]:
                self.error("records[%d][3]='%s'" % (i, rec[1]))
            if 'A1' == rec[0] and "A1-1m,int 1m,0.1m Yellow" != rec[1]:
                self.error("records[%d][3]='%s'" % (i, rec[1]))
            if 'A2' == rec[0] and "A2 -1m,int 1m,0.2m White" != rec[1]:
                self.error("records[%d][3]='%s'" % (i, rec[1]))
            if 'A3' == rec[0] and "A3 -1m,int 1m,0.3m yellow" != rec[1]:
                self.error("records[%d][3]='%s'" % (i, rec[1]))
            if 'A4' == rec[0] and "A4 - broken white lines at signalised pedestrian crossing" != rec[1]:
                self.error("records[%d][3]='%s'" % (i, rec[1]))
            if 'B' == rec[0] and "B - 2m,int 4m,0.1m white" != rec[1]:
                self.error("records[%d][3]='%s'" % (i, rec[1]))
            if 'B1' == rec[0] and "B1 - 2m,int 10m ,0.1m White" != rec[1]:
                self.error("records[%d][3]='%s'" % (i, rec[1]))
            if 'C' == rec[0] and "C - 4m,int 2m,0.1m white" != rec[1]:
                self.error("records[%d][3]='%s'" % (i, rec[1]))
            if 'D' == rec[0] and "D - 1m,int 1m,0.1m dbl white" != rec[1]:
                self.error("records[%d][3]='%s'" % (i, rec[1]))
            if 'E' == rec[0] and "E -2.75m,int 2.75m,0.15m white" != rec[1]:
                self.error("records[%d][3]='%s'" % (i, rec[1]))
            if 'F' == rec[0] and "F - Centre line,0.15 m white" != rec[1]:
                self.error("records[%d][3]='%s'" % (i, rec[1]))
            if 'I' == rec[0] and "I - continuous Double Yellow Line" != rec[1]:
                self.error("records[%d][3]='%s'" % (i, rec[1]))
            if 'G' == rec[0] and "G - Side Line,0.15 yellow" != rec[1]:
                self.error("records[%d][3]='%s'" % (i, rec[1]))
            if 'H' == rec[0] and "H - continuous Double White Line" != rec[1]:
                self.error("records[%d][3]='%s'" % (i, rec[1]))
            if 'J' == rec[0] and "J - White Line-Ped Crossing" != rec[1]:
                self.error("records[%d][3]='%s'" % (i, rec[1]))
            if 'K' == rec[0] and "K - Zig Zag Line" != rec[1]:
                self.error("records[%d][3]='%s'" % (i, rec[1]))
            if 'L' == rec[0] and "L - Yellow Line - bus lane" != rec[1]:
                self.error("records[%d][3]='%s'" % (i, rec[1]))
            if 'M' == rec[0] and "M - White Line-stop line" != rec[1]:
                self.error("records[%d][3]='%s'" % (i, rec[1]))
            if 'N' == rec[0] and "N - Yellow Box Line" != rec[1]:
                self.error("records[%d][3]='%s'" % (i, rec[1]))
            if 'O' == rec[0] and "O - Single yellow zig-zag" != rec[1]:
                self.error("records[%d][3]='%s'" % (i, rec[1]))
            if 'P' == rec[0] and "P - Double yellow zig-zag" != rec[1]:
                self.error("records[%d][3]='%s'" % (i, rec[1]))
            if 'Q' == rec[0] and "Q - Multi-arrow" != rec[1]:
                self.error("records[%d][3]='%s'" % (i, rec[1]))
            if 'R' == rec[0] and "R - Bus Zone" != rec[1]:
                self.error("records[%d][3]='%s'" % (i, rec[1]))
            if 'S' == rec[0] and "S - continuous red line for full day bus lane" != rec[1]:
                self.error("records[%d][3]='%s'" % (i, rec[1]))
            if 'S1' == rec[0] and "S1 - Dotted red line for full day bus lane" != rec[1]:
                self.error("records[%d][3]='%s'" % (i, rec[1]))
            if 'T' == rec[0] and "T- Turning Pocket" != rec[1]:
                self.error("records[%d][3]='%s'" % (i, rec[1]))
            if 'U' == rec[0] and "U - Pedestrian crossing ahead marking" != rec[1]:
                self.error("records[%d][3]='%s'" % (i, rec[1]))

            shape = shapes[i]
            if 3 != shape.shapeType:
                self.error("shapes[%d].shapeType=%d" % (i, shape.shapeType))
            if 1 != len(shape.parts) and 0 != shape.parts[0]:
                self.error("shapes[%d].parts=%s" % (i, shape.parts))
            polyline = list()
            for point in shape.points:
                polyline.append(Point(point[0], point[1]))

            self.lane_markings.append(Lane_marking(i, type, polyline))

    def records(self):
        return self.lane_markings

if "__main__" == __name__:
    def error():
        print "Usage: %s data-folder" % sys.argv[0]
        print "must specify the folder containing the lane-markings shape files"
        sys.exit(1)

    import sys
    if len(sys.argv) != 2:
        error()

    try:
        lane_markings = Lane_markings("%s/LaneMarking_15Sep10_104241" % sys.argv[1])
    except shapefile.ShapefileException as reason:
        print reason
        error()

    print "bounding box =", lane_markings.bounding_box
    for lane_marking in lane_markings.lane_markings:
        print lane_marking
