#!/usr/bin/env python

# Copyright Singapore-MIT Alliance for Research and Technology

from uninode import Uni_node
from multinode import Multi_node
from tsignal import Traffic_signal
from link import Link
from segment import Road_segment
from polyline import Polyline
from lane import Lane
from crossing import Crossing
from connector import Connector
from circular import Circular

class Network:
    def __init__(self):
        self.road_items = dict()
        self.road_segments = list()
        self.crossings = list()

    def parse(self, line):
        if Uni_node.parse(line, self.road_items):
            return True
        if Multi_node.parse(line, self.road_items):
            return True
        if Traffic_signal.parse(line, self.road_items):
            return True
        if Link.parse(line, self.road_items):
            return True
        if Road_segment.parse(line, self.road_items, self.road_segments):
            return True
        if Polyline.parse(line, self.road_items):
            return True
        if Lane.parse(line, self.road_items):
            return True
        if Crossing.parse(line, self.road_items, self.crossings):
            return True
        if Connector.parse(line, self.road_items):
            return True
        #if Circular.parse(line, self.road_items):
        #    return True
        return False

    def resolve(self):
        for item in self.road_items.values():
            item.resolve(self.road_items)

if "__main__" == __name__:
    input_file = open("../run.log")
    network = Network()
    for line in input_file:
        network.parse(line)
    input_file.close()
    network.resolve()
    for pointer, item in network.road_items.iteritems():
        print "%s -> %s" % (pointer, item)
