#!/usr/bin/env python

# Copyright Singapore-MIT Alliance for Research and Technology

import re, json
from error import Error

def json_hook(dictionary):
    at_node = dictionary["at-node"]
    at_segment = dictionary["at-segment"]
    is_forward = True if int(dictionary["fwd"]) == 1 else False
    number = int(dictionary["number"])
    return Circular(at_node, at_segment, is_forward, number)

class Circular:
    regexp = re.compile('\("tmp-circular", 0, 0, {(.+),}\)')

    @staticmethod
    def parse(line, road_items):
        mo = Circular.regexp.search(line)
        if not mo:
            return False

        pointer = mo.group(1)
        if pointer in road_items:
            raise Error("circular: %s already exists in road_items" % pointer)
        data = '{' + mo.group(2) + '}' 
        road_items[pointer] = json.loads(data, object_hook=json_hook)
        return True

    def __init__(self, at_node, at_segment, is_forward, number):
        self.at_node = at_node
        self.at_segment = at_segment
        self.is_forward = is_forward
        self.number = number

    def resolve(self, road_items):
        if self.at_node not in road_items:
            raise Error("circular: unknown node %s" % self.at_node)
        if self.at_segment not in road_items:
            raise Error("circular: unknown node %s" % self.at_segment)
        self.at_node = road_items[self.at_node]
        self.at_segment = road_items[self.at_segment]

    def __repr__(self):
        return "circular"
