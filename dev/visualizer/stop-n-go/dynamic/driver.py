#!/usr/bin/env python

# Copyright Singapore-MIT Alliance for Research and Technology

import re, json
from static.point import Point

def json_hook(dictionary):
    x = int(dictionary["xPos"])
    y = int(dictionary["yPos"])
    position = Point(x, y)
    angle = float(dictionary["angle"])
    length = int(dictionary["length"])
    width = int(dictionary["width"])
    return position, angle, length, width

class Driver:
    regexp = re.compile('\("Driver",(\d+),(\d+),(.+)\)')

    @staticmethod
    def parse(line, agents, animator):
        mo = Driver.regexp.search(line)
        if not mo:
            return False

        frame_number = int(mo.group(1))
        id = int(mo.group(2))
        if id not in agents:
            agents[id] = Driver(id)
        driver = agents[id]

        position, angle, length, width = json.loads(mo.group(3), object_hook=json_hook)
        driver.set_extent(length, width)
        animator.set_driver_position_and_orientation_at(driver, frame_number, position, angle)

    def __init__(self, id):
        self.id = id
        self.length = 0
        self.width = 0

    def set_extent(self, length, width):
        if self.length != length:
            if self.length:
                print "driver %d is changing vehicle" % self.id
            self.length = length
        if self.width != width:
            if self.width:
                print "driver %d is changing vehicle" % self.id
            self.width = width

    def __repr__(self):
        return "driver(%d)" % self.id
