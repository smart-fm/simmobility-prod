#!/usr/bin/env python

# Copyright Singapore-MIT Alliance for Research and Technology

import re, json
import point
from error import Error

def json_hook(dictionary):
    road_segment = dictionary["parent-segment"]
    lane_edges = dict()
    index = 0
    while True:
        key = "line-%d" % index
        if key not in dictionary:
            break
        string = dictionary[key]
        points = point.get_polyline(string)
        is_side_walk = True if key + "is-sidewalk" in dictionary else False
        lane_edges[index] = Lane_edge(points, is_side_walk)
        index += 1
    return Lane(road_segment, lane_edges)

class Lane_edge:
    def __init__(self, points, is_side_walk):
        self.points = points
        self.is_side_walk = is_side_walk

class Lane:
    regexp = re.compile('\("lane", 0, (0x[0-9a-fA-F]+), {(.*),}\)')

    @staticmethod
    def parse(line, road_items):
        mo = Lane.regexp.search(line)
        if not mo:
            return False

        pointer = mo.group(1)
        if pointer in road_items:
            raise Error("lane: %s already exists in road_items" % pointer)
        data = '{' + mo.group(2) + '}' 
        road_items[pointer] = json.loads(data, object_hook=json_hook)
        return True

    def __init__(self, road_segment, lane_edges):
        self.road_segment = road_segment
        self.lane_edges = lane_edges

    def resolve(self, road_items):
        if self.road_segment not in road_items:
            raise Error("lane: unknown road-segment %s" % self.road_segment)
        self.road_segment = road_items[self.road_segment]
        self.road_segment.set_lane_edges(self.lane_edges)

    def __repr__(self):
        return "lane(in %s)" % self.road_segment
