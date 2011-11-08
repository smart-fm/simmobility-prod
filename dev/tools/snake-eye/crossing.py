#!/usr/bin/env python

# Copyright Singapore-MIT Alliance for Research and Technology

import re
from point import Point

class Crossing(Point):
    regexp = re.compile("aimsun::crossing x=(\d+), y=(\d+), section-id=(\d+), lane-id=(\d+), lane-type='([^']+)'")

    def __init__(self, x, y, section, lane, lane_type):
        super(Crossing, self).__init__(x, y)
        self.section = section
        self.lane = lane
        self.lane_type = lane_type

    def __repr__(self):
        return "crossing"

    @staticmethod
    def check(line, crossings):
        mo = Crossing.regexp.search(line)
        if not mo:
            return False
        crossings.append(Crossing(int(mo.group(1)), int(mo.group(2)), mo.group(3),
                                  mo.group(4), mo.group(5)))
        return True
