#!/usr/bin/env python

# Copyright Singapore-MIT Alliance for Research and Technology

import re, json
import point
from error import Error

def json_hook(dictionary):
    road_segment = dictionary["parent-segment"]
    string = dictionary["points"]
    points = point.get_polyline(string)
    return Polyline(road_segment, points)

class Polyline:
    regexp = re.compile('\("polyline", 0, (0x[0-9a-fA-F]+), {(.+),}\)')

    @staticmethod
    def parse(line, road_items):
        mo = Polyline.regexp.search(line)
        if not mo:
            return False

        pointer = mo.group(1)
        if pointer in road_items:
            raise Error("polyline: %s already exists in road_items" % pointer)
        data = '{' + mo.group(2) + '}' 
        road_items[pointer] = json.loads(data, object_hook=json_hook)
        return True

    def __init__(self, road_segment, points):
        self.road_segment = road_segment
        self.points = points

    def resolve(self, road_items):
        if self.road_segment not in road_items:
            raise Error("polyline: unknown road-segment %s" % self.road_segment)
        self.road_segment = road_items[self.road_segment]
        self.road_segment.set_polyline(self.points)

    def __repr__(self):
        return "polyline(in %s)" % self.road_segment
