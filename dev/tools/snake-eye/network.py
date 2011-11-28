#!/usr/bin/env python

# Copyright Singapore-MIT Alliance for Research and Technology

"""Extract the data from aimsun.log file.  The simulator was modified to dump whatever it
retrieve from the SimMobility database into the aimsun.log file.  This module should be
replaced by database.py which retrieves the data directly from the database."""

from node import Node
from section import Section
from crossing import Crossing
from lane import Lane
from turning import Turning
from polyline import Polyline

class Network:
    def __init__(self, filename):
        self.things = dict()
        self.roads = dict()
        self.crossings = list()
        self.lanes = list()
        self.polylines = list()

        f = open(filename, 'r')
        for line in f:
            if Node.check(line, self.things):
                continue
            elif Section.check(line, self.things, self.roads):
                continue
            elif Crossing.check(line, self.crossings):
                continue
            elif Lane.check(line, self.lanes):
                continue
            elif Turning.check(line, self.things):
                continue
            elif Polyline.check(line, self.polylines):
                continue

        self.post_process()

    def post_process(self):
        self.process_sections()
        self.process_crossings()
        self.process_lanes()
        self.process_polylines()

    def process_sections(self):
        for key, item in self.things.iteritems():
            if isinstance(item, Section):
                item.resolve(self.things)

    def process_crossings(self):
        for point in self.crossings:
            if point.section not in self.things:
                raise "can't find section %s in things for crossings" % point.section
            section = self.things[point.section]
            section.add_crossing(point.lane, point)

    def process_lanes(self):
        for point in self.lanes:
            if point.section not in self.things:
                raise "can't find section %s in things for lanes" % point.section
            section = self.things[point.section]
            section.add_lane(point.lane, point)

    def process_polylines(self):
        for point in self.polylines:
            if point.section not in self.things:
                raise "can't find section %s in things for polylines" % point.section
            section = self.things[point.section]
            section.add_point_to_polyline(point)

    def dump(self):
        for key, item in self.things.iteritems():
            print "%s -> %s" % (key, item)
