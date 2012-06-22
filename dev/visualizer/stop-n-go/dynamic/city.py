#!/usr/bin/env python

# Copyright Singapore-MIT Alliance for Research and Technology

from driver import Driver
from tsignal import Traffic_signal
from pedestrian import Pedestrian

class City:
    def __init__(self, animator):
        self.agents = dict()
        self.animator = animator

    def parse(self, line):
        if Driver.parse(line, self.agents, self.animator):
            return True
        if Traffic_signal.parse(line, self.agents, self.animator):
            return True
        if Pedestrian.parse(line, self.agents, self.animator):
            return True
        return False

if "__main__" == __name__:
    input_file = open("../run.log")
    city = City()
    for line in input_file:
        city.parse(line)
    input_file.close()
    for id, agent in city.agents.iteritems():
        print "%s -> %s" % (str(id), agent)
