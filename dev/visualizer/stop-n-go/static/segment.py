#!/usr/bin/env python

# Copyright Singapore-MIT Alliance for Research and Technology

import re, json
from error import Error

def json_hook(dictionary):
    link = dictionary["parent-link"]
    max_speed = int(dictionary["max-speed"])
    lane_count = int(dictionary["lanes"])
    from_node = dictionary["from-node"]
    to_node = dictionary["to-node"]
    db_id = int(dictionary["aimsun-id"])
    return Road_segment(link, max_speed, lane_count, from_node, to_node, db_id)

class Road_segment:
    regexp = re.compile('\("road-segment", 0, (0x[0-9a-fA-F]+), {(.+),}\)')

    @staticmethod
    def parse(line, road_items, road_segments):
        mo = Road_segment.regexp.search(line)
        if not mo:
            return False

        pointer = mo.group(1)
        if pointer in road_items:
            raise Error("road-segment: %s already exists in road_items" % pointer)
        data = '{' + mo.group(2) + '}' 
        road_segment = json.loads(data, object_hook=json_hook)
        road_items[pointer] = road_segment
        road_segments.append(road_segment)
        return True

    def __init__(self, link, max_speed, lane_count, from_node, to_node, db_id):
        self.link = link
        self.max_speed = max_speed
        self.lane_count = lane_count
        self.from_node = from_node
        self.to_node = to_node
        self.db_id = db_id

    def resolve(self, road_items):
        if self.link not in road_items:
            raise Error("road-segment: unknown link %s" % self.link)
        if self.from_node not in road_items:
            raise Error("road-segment: unknown from-node %s" % self.from_node)
        if self.to_node not in road_items:
            raise Error("road-segment: unknown to-node %s" % self.from_node)
        self.link = road_items[self.link]
        self.from_node = road_items[self.from_node]
        self.to_node = road_items[self.to_node]

    def set_road_name(self, road_name):
        self.road_name = road_name

    def set_polyline(self, polyline):
        self.polyline = polyline

    def set_lane_edges(self, lane_edges):
        self.lane_edges = lane_edges

    def __repr__(self):
        return "road-segment(from %s to %s)" % (self.from_node, self.to_node)
