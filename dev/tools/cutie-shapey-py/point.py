#!/usr/bin/python

# Copyright Singapore-MIT Alliance for Research and Technology

import collections

Point = collections.namedtuple("Point", ['x', 'y'])
class Point:
    def __init__(self, x, y):
        """Must pass floating-point arguments to the constructor in meters.
        The point co-ordinates are stored as integers in centimeters."""
        if not isinstance(x, float) or not isinstance(y, float):
            raise ValueError("Point takes on floating-point arguments")
        self.x = int(x * 100)
        self.y = int(y * 100)

    def __repr__(self):
        return "(%d, %d)" % (self.x, self.y)

Bounding_box = collections.namedtuple("Bounding_box", ["lower_left", "upper_right"])
