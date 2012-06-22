#!/usr/bin/env python

# Copyright Singapore-MIT Alliance for Research and Technology

import re, json
from static.point import Point

def json_hook(dictionary):
    x = int(dictionary["xPos"])
    y = int(dictionary["yPos"])
    return Point(x, y)

class Pedestrian:
    regexp = re.compile('\("pedestrian",(\d+),(\d+),{(.+),}\)')

    @staticmethod
    def parse(line, agents, animator):
        mo = Pedestrian.regexp.search(line)
        if not mo:
            return False

        frame_number = int(mo.group(1))
        id = int(mo.group(2))
        if id not in agents:
            agents[id] = Pedestrian(id)
        pedestrian = agents[id]

        data = '{' + mo.group(3) + '}'
        position = json.loads(data, object_hook=json_hook)
        animator.set_pedestrian_position_at(pedestrian, frame_number, position)

    def __init__(self, id):
        self.id = id

    def __repr__(self):
        return "pedestrian(%d)" % self.id
