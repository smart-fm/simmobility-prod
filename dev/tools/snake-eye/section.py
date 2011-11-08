#!/usr/bin/env python

# Copyright Singapore-MIT Alliance for Research and Technology

import re

class Section:
    regexp = re.compile("aimsun::section id=(\d+), from-node=(\d+), to-node=(\d+), length=(\d+), speed=(\d+), num-lanes=(\d+), capacity=(\d+), name='([^']+)'")

    def __init__(self, from_node, to_node, length, speed, num_lanes, capacity, name, id):
        self.from_node_id = from_node
        self.to_node_id = to_node
        self.from_node = None
        self.to_node = None
        self.length = length
        self.speed = speed
        self.num_lanes = num_lanes
        self.capacity = capacity
        self.name = name
        self.id = id
        self.polyline = list()
        self.lanes = dict()
        self.crossings = dict()

    def resolve(self, things):
        if self.from_node_id not in things:
            print "can't find from-node %s for road-name='%s'" % (self.from_node_id, self.name)
        else:
            self.from_node = things[self.from_node_id]
        if self.to_node_id not in things:
            print "can't find to-node %s for road-name='%s'" % (self.to_node_id, self.name)
        else:
            self.to_node = things[self.to_node_id]

    def add_point_to_polyline(self, point):
        self.polyline.append(point)

    def add_lane(self, lane, point):
        if lane not in self.lanes:
            self.lanes[lane] = dict()
        if point.lane_type not in self.lanes[lane]:
            self.lanes[lane][point.lane_type] = list()
        self.lanes[lane][point.lane_type].append(point)

    def add_crossing(self, lane, point):
        if lane not in self.crossings:
            self.crossings[lane] = dict()
        if point.lane_type not in self.crossings[lane]:
            self.crossings[lane][point.lane_type] = list()
        self.crossings[lane][point.lane_type].append(point)

    def __repr__(self):
        return "section"

    @staticmethod
    def check(line, things, roads):
        mo = Section.regexp.search(line)
        if not mo:
            return False
        if mo.group(1) in things:
            raise "section %s already found" % mo.group(1)
        section = Section(mo.group(2), mo.group(3), int(mo.group(4)), int(mo.group(5)),
                          int(mo.group(6)), int(mo.group(7)), mo.group(8), mo.group(1))
        things[mo.group(1)] = section
        if section.name not in roads:
            roads[section.name] = list()
        roads[section.name].append(section)
        return True
