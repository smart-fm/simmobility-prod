#!/usr/bin/env python

# Copyright Singapore-MIT Alliance for Research and Technology

import re, json
from point import Point

def json_hook(dictionary):
    point1 = get_point(dictionary["near-1"])
    point2 = get_point(dictionary["near-2"])
    near = (point1, point2)
    point1 = get_point(dictionary["far-1"])
    point2 = get_point(dictionary["far-2"])
    far = (point1, point2)
    return Crossing(near, far)

def get_point(string):
    x, y = string.split(',')
    return Point(int(x), int(y))

class Crossing:
    regexp = re.compile('\("crossing", 0, (0x[0-9a-fA-F]+), {(.+),}\)')

    @staticmethod
    def parse(line, road_items, crossings):
        mo = Crossing.regexp.search(line)
        if not mo:
            return False

        pointer = mo.group(1)
        if pointer in road_items:
            raise Error("crossing: %s already exists in road_items" % pointer)
        data = '{' + mo.group(2) + '}' 
        crossing = json.loads(data, object_hook=json_hook)
        road_items[pointer] = crossing
        crossings.append(crossing)
        return True

    def __init__(self, near, far):
        self.near = near
        self.far = far

    def resolve(self, road_items):
        pass

    def __repr__(self):
        return "crossing"
