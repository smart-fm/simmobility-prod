#!/usr/bin/env python

# Copyright Singapore-MIT Alliance for Research and Technology

from point import Point

class Kerb_line:
    def __init__(self, id, lane_id, type, section_id):
        self.id = id
        self.lane_id = lane_id
        self.type = type
        self.section_id = section_id

        self.polyline = list()

    def add_point(self, x, y, row_number):
        self.polyline.append(Point(x, y, row_number))

    def sort_polyline(self):
        self.polyline.sort(key = lambda point : point.row_number)
