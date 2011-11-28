#!/usr/bin/env python

# Copyright Singapore-MIT Alliance for Research and Technology

import re
from point import Point

class Polyline(Point):
    regexp = re.compile("aimsun::polyline section-id=(\d+), x=(\d+), y=(\d+)")

    def __init__(self, x, y, section):
        super(Polyline, self).__init__(x, y)
        self.section = section

    def __repr__(self):
        return "polyline"

    @staticmethod
    def check(line, polylines):
        mo = Polyline.regexp.search(line)
        if not mo:
            return False
        polylines.append(Polyline(int(mo.group(2)), int(mo.group(3)), mo.group(1)))
        return True
