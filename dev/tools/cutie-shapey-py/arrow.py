#!/usr/bin/python

# Copyright Singapore-MIT Alliance for Research and Technology

import shapefile
from point import Point, Bounding_box
from PyQt4 import QtGui, QtCore

class Arrow_marking:
    def __init__(self, id, position, bearing, type):
        self.id = id
        self.position = position
        self.bearing = bearing
        self.type = type

    def __repr__(self):
        return "Arrow-marking id=%d position=%s bearing=%d type=%s" % (self.id, self.position,
                                                                       self.bearing, self.type)

    @staticmethod
    def column_names():
        return ("#", "Type", "Bearing", "Shape-type", "Position")

    def columns(self):
        return (self.id, self.type, self.bearing, "point", self.position)

    def tip_columns(self):
        return [0]

    def tips(self):
        return [self.type_desc()]

    def type_desc(self):
        if 'A' == self.type: return "Right Turn"
        if 'B' == self.type: return "St/Lt Turn"
        if 'C' == self.type: return "St/Rt Turn"
        if 'D' == self.type: return "Straight"
        if 'E' == self.type: return "Lt Turn"
        if 'F' == self.type: return "Lt/Rt Turn"
        if 'G' == self.type: return "Lt Conv"
        if 'H' == self.type: return "Rt Conv"
        if 'I' == self.type: return "St/Lt/Rt Turn"

    def graphics(self):
        if 'A' == self.type:
            path = self.right_arrow()
        elif 'B' == self.type:
            path = self.straight_and_left_arrow()
        elif 'C' == self.type:
            path = self.straight_and_right_arrow()
        elif 'D' == self.type:
            path = self.straight_arrow()
        elif 'E' == self.type:
            path = self.left_arrow()
        elif 'F' == self.type:
            path = self.left_and_right_arrow()
        elif 'G' == self.type:
            path = self.conv_left_arrow()
        elif 'H' == self.type:
            path = self.conv_right_arrow()
        elif 'I' == self.type:
            path = self.straight_and_left_and_right_arrow()
        else:
            path = None
        if not path:
            return None
        item = QtGui.QGraphicsPathItem(path)
        item.setBrush(QtCore.Qt.green)
        item.setPos(self.position.x, self.position.y)
        item.setRotation(-self.bearing)
        item.info = "arrow marking id=%d type='%s' bearing=%d" % (self.id, self.type_desc(), self.bearing)
        return item

    def arrow_stem(self):
        path = QtGui.QPainterPath()
        path.addRect(-15, 0, 30, 240)
        return path

    def arrow_head(self):
        path = QtGui.QPainterPath(QtCore.QPointF(-15, 0))
        path.lineTo(15, 0)
        path.lineTo(15, 60)
        path.lineTo(45, 60)
        path.lineTo(0, 120)
        path.lineTo(-45, 60)
        path.lineTo(-15, 60)
        path.closeSubpath()
        return path

    def rotated_arrow_head(self, angle):
        matrix = QtGui.QTransform()
        matrix.rotate(angle)
        path = self.arrow_head()
        polygon = path.toFillPolygon(matrix)
        path = QtGui.QPainterPath()
        path.addPolygon(polygon)
        return path

    def right_arrow(self):
        path1 = self.arrow_stem()
        path2 = self.rotated_arrow_head(-90)
        path2.translate(-15, 225)
        return path1.united(path2)

    def straight_and_left_arrow(self):
        path1 = self.straight_arrow()
        path2 = self.rotated_arrow_head(90)
        path2.translate(15, 150)
        return path1.united(path2)

    def straight_and_right_arrow(self):
        path1 = self.straight_arrow()
        path2 = self.rotated_arrow_head(-90)
        path2.translate(-15, 150)
        return path1.united(path2)

    def straight_arrow(self):
        path1 = self.arrow_stem()
        path2 = self.arrow_head()
        path2.translate(0, 180)
        return path1.united(path2)

    def left_arrow(self):
        path1 = self.arrow_stem()
        path2 = self.rotated_arrow_head(90)
        path2.translate(15, 225)
        return path1.united(path2)

    def left_and_right_arrow(self):
        path1 = self.left_arrow()
        path2 = self.right_arrow()
        return path1.united(path2)

    def conv_right_arrow(self):
        # At junction of Rochor Canal Road and Queen Street.
        path = QtGui.QPainterPath(QtCore.QPointF(0, 0))
        rect = QtCore.QRectF(-45, 0, 90, 210)
        path.arcTo(rect, -90, 180)
        path.lineTo(0, 240)
        path.lineTo(-60, 195)
        path.lineTo(0, 150)
        path.lineTo(0, 180)
        rect = QtCore.QRectF(-30, 30, 60, 150)
        path.arcTo(rect, 90, -180)
        path.closeSubpath()
        return path

    def conv_left_arrow(self):
        # At junction of Rochor Canal Road and Queen Street.
        path = QtGui.QPainterPath(QtCore.QPointF(0, 0))
        rect = QtCore.QRectF(-45, 0, 90, 210)
        path.arcTo(rect, -90, -180)
        path.lineTo(0, 240)
        path.lineTo(60, 195)
        path.lineTo(0, 150)
        path.lineTo(0, 180)
        rect = QtCore.QRectF(-30, 30, 60, 150)
        path.arcTo(rect, 90, 180)
        path.closeSubpath()
        return path

    def straight_and_left_and_right_arrow(self):
        # Found at junction of Short Street, Albert Street, and McNally Street (???). 
        path1 = self.straight_and_left_arrow()
        path2 = self.straight_and_right_arrow()
        return path1.united(path2)

class Arrow_markings:
    def __init__(self, shape_name):
        sf = shapefile.Reader(shape_name)

        box = sf.bbox
        lower_left = Point(box[0], box[1])
        upper_right = Point(box[2], box[3])
        self.bounding_box = Bounding_box(lower_left, upper_right)

        self.arrow_markings = list()
        self.process_records_and_shapes(sf.numRecords, sf.records(), sf.shapes())

    @staticmethod
    def column_names():
        return Arrow_marking.column_names()

    def error(self, message):
        print "error in arrow-markings: %s" % message

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

            # The 1st field is BEARG_NUM, a floating-point number.  I think this is the angle
            # (bearing).  At this moment, I don't know if this angle is with respect to the X-axis
            # or some other axis.  I also don't know what it means.  I suspect if shows the
            # direction of the lane connector.  I suspect the point in the corresponding shape
            # structure will give the position in the lane for this arrow marking.  The bearing
            # gives the direction of the marking, hence pointing to the lane.  But what if the
            # arrow is for two directions.....
            if not isinstance(rec[0], float):
                self.error("records[%d][0]='%s'" % (i, str(rec[0])))
            bearing = int(rec[0])

            # The 2nd field is BEARG_NUM_, a string of 254 characters.  I think the fields in all
            # records are just strings of 254 <SPACE> character.  If that is the case, remove
            # the field.
            if not self.is_all_blanks(rec[1]):
                self.error("records[%d][1]='%s'" % (i, str(rec[1])))

            # The 3rd field is TYPE_CD, a string of 4 characters.  So far, I only see 6 types,
            # all consisting a single upper-case letter.
            if rec[2] not in "ABCDEFGHI":
                self.error("records[%d][2]='%s'" % (i, rec[2]))
            type = rec[2]

            if 'A' == rec[2] and "A - Right Turn" != rec[3]:
                self.error("records[%d][3]='%s'" % (i, rec[3]))
            if 'B' == rec[2] and "B - St/Lt Turn" != rec[3]:
                self.error("records[%d][3]='%s'" % (i, rec[3]))
            if 'C' == rec[2] and "C - St/Rt Turn" != rec[3]:
                self.error("records[%d][3]='%s'" % (i, rec[3]))
            if 'D' == rec[2] and "D - Straight" != rec[3]:
                self.error("records[%d][3]='%s'" % (i, rec[3]))
            if 'E' == rec[2] and "E - Lt Turn" != rec[3]:
                self.error("records[%d][3]='%s'" % (i, rec[3]))
            if 'F' == rec[2] and "F - Lt/Rt Turn" != rec[3]:
                self.error("records[%d][3]='%s'" % (i, rec[3]))
            if 'G' == rec[2] and "G - Lt Conv" != rec[3]:
                self.error("records[%d][3]='%s'" % (i, rec[3]))
            if 'H' == rec[2] and "H - Rt Conv" != rec[3]:
                self.error("records[%d][3]='%s'" % (i, rec[3]))
            if 'I' == rec[2] and "I - St/Lt/Rt Turn" != rec[3]:
                self.error("records[%d][3]='%s'" % (i, rec[3]))

            shape = shapes[i]
            if 1 != shape.shapeType:
                self.error("shapes[%d].shapeType=%d" % (i, shape.shapeType))
            if 1 != len(shape.points):
                self.error("shapes[%d].points has %d items" % (i, len(shape.points)))

            point = shape.points[0]
            position = Point(point[0], point[1])

            self.arrow_markings.append(Arrow_marking(i, position, bearing, type))

    def records(self):
        return self.arrow_markings

if "__main__" == __name__:
    def error():
        print "Usage: %s data-folder" % sys.argv[0]
        print "must specify the folder containing the arrow-markings shape files"
        sys.exit(1)

    import sys
    if len(sys.argv) != 2:
        error()

    try:
        arrow_markings = Arrow_markings("%s/ArrowMarking_15Sep10_104237" % sys.argv[1])
    except shapefile.ShapefileException as reason:
        print reason
        error()

    print "bounding box =", arrow_markings.bounding_box
    for arrow_marking in arrow_markings.arrow_markings:
        print arrow_marking
