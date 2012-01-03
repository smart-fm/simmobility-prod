#!/usr/bin/env python

# Copyright Singapore-MIT Alliance for Research and Technology

from lane import Lane_marking
from kerb import Kerb_line

class Section:
    def __init__(self, id, road, number_lanes, speed_limit, capacity, from_node, to_node, length):
        self.id = id
        self.name = road
        self.number_lanes = number_lanes
        self.speed_limit = speed_limit
        self.capacity = capacity
        self.from_node = from_node
        self.to_node = to_node
        self.length = length

        self.lane_markings = dict()
        self.kerb_lines = dict()

    def add_lane_marking_point(self, lane_id, type, x, y, row_number):
        if lane_id not in self.lane_markings:
            lane_marking = Lane_marking(lane_id, type, self.id)
            self.lane_markings[lane_id] = lane_marking
        lane_marking = self.lane_markings[lane_id]
        lane_marking.add_point(x, y, row_number)

    def add_kerb_line_point(self, kerb_id, lane_id, type, x, y, row_number):
        if kerb_id not in self.kerb_lines:
            self.kerb_lines[kerb_id] = Kerb_line(kerb_id, lane_id, type, self.id)
        kerb_line = self.kerb_lines[kerb_id]
        kerb_line.add_point(x, y, row_number)
