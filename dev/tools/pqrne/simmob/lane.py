#!/usr/bin/env python

# Copyright Singapore-MIT Alliance for Research and Technology

from point import Point

class Lane_marking:
    def __init__(self, id, type, section_id):
        self.id = id
        self.type = type
        self.section_id = section_id

        self.polyline = list()

    def add_point(self, x, y, row_number):
        self.polyline.append(Point(x, y, row_number))

    def sort_polyline(self):
        self.polyline.sort(key = lambda point : point.row_number)
