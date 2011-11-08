#!/usr/bin/env python

# Copyright Singapore-MIT Alliance for Research and Technology

import re

class Turning:
    regexp = re.compile("aimsun::turning id=(\d+), from-lane-a=(\d+), from-lane-b=(\d+), to-lane-a=(\d+), to-lane-b=(\d+), from-section=(\d+), to-section=(\d+)")

    def __init__(self, from_section, to_section, from_lane_a, from_lane_b, to_lane_a, to_lane_b):
        self.from_section = from_section
        self.to_section = to_section
        self.from_lane_a = from_lane_a
        self.from_lane_b = from_lane_b
        self.to_lane_a = to_lane_a
        self.to_lane_b = to_lane_b

    def __repr__(self):
        return "turning"

    @staticmethod
    def check(line, things):
        mo = Turning.regexp.search(line)
        if not mo:
            return False
        if mo.group(1) in things:
            raise "turning %s already found" % mo.group(1)
        things[mo.group(1)] = Turning(mo.group(6), mo.group(7), int(mo.group(2)),
                                      int(mo.group(3)), int(mo.group(4)), int(mo.group(5)))
        return True
