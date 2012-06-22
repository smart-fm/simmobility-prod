#!/usr/bin/env python

# Copyright Singapore-MIT Alliance for Research and Technology

import re, json
from error import Error

def json_hook(dictionary):
    from_segment = dictionary["from-segment"]
    from_lane = int(dictionary["from-lane"])
    to_segment = dictionary["to-segment"]
    to_lane = int(dictionary["to-lane"])
    return Connector(from_segment, from_lane, to_segment, to_lane)

class Connector:
    regexp = re.compile('\("lane-connector", 0, (0x[0-9a-fA-F]+), {(.+),}\)')

    @staticmethod
    def parse(line, road_items):
        mo = Connector.regexp.search(line)
        if not mo:
            return False

        pointer = mo.group(1)
        if pointer in road_items:
            raise Error("connector: %s already exists in road_items" % pointer)
        data = '{' + mo.group(2) + '}' 
        road_items[pointer] = json.loads(data, object_hook=json_hook)
        return True

    def __init__(self, from_segment, from_lane, to_segment, to_lane):
        self.from_segment = from_segment
        self.from_lane = from_lane
        self.to_segment = to_segment
        self.to_lane = to_lane

    def resolve(self, road_items):
        if self.from_segment not in road_items:
            raise Error("connector: unknown from-segment %s" % self.from_segment)
        if self.to_segment not in road_items:
            raise Error("connector: unknown to-segment %s" % self.to_segment)
        self.from_segment = road_items[self.from_segment]
        self.to_segment = road_items[self.to_segment]

    def __repr__(self):
        return "connector"
