#!/usr/bin/python

# Copyright Singapore-MIT Alliance for Research and Technology

import shapefile
from point import Point, Bounding_box

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
        return ("Type", "Bearing", "Shape-type", "Position")

    def columns(self):
        return (self.type, self.bearing, "point", self.position)

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
        return None

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
        print "error in bus-stop: %s" % message

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
