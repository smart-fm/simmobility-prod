#!/usr/bin/env python

# Copyright Singapore-MIT Alliance for Research and Technology

import re

def get_polyline(string):
    points = list()
    start = 0
    while True:
        mo = Point.regexp.search(string, start)
        if not mo:
            break
        points.append(Point(int(mo.group(1)), int(mo.group(2))))
        start = mo.end(0)
    return points

class Point(object):
    regexp = re.compile('\((\d+),(\d+)\),')

    def __init__(self, x, y):
        self.x = x
        self.y = y

    def __repr__(self):
        return "(%d, %d)" % (self.x, self.y)
