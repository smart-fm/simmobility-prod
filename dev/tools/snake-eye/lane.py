#!/usr/bin/env python

# Copyright Singapore-MIT Alliance for Research and Technology

import re
from point import Point

class Lane(Point):
    regexp = re.compile("aimsun::lane lane-id=(\d+), lane-type='([^']+)', x=(\d+), y=(\d+), section-id=(\d+)")

    def __init__(self, x, y, section, lane, lane_type):
        super(Lane, self).__init__(x, y)
        self.section = section
        self.lane = lane
        self.lane_type = lane_type

    def __repr__(self):
        return "lane"

    @staticmethod
    def check(line, lanes):
        mo = Lane.regexp.search(line)
        if not mo:
            return False
        lanes.append(Lane(int(mo.group(3)), int(mo.group(4)), mo.group(5),
                              mo.group(1), mo.group(2)))
        return True
