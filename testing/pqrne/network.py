#!/usr/bin/env python

# Copyright Singapore-MIT Alliance for Research and Technology

import os, bz2, pickle

class Road_network:
    def __init__(self, save_file_name):
        self.nodes = dict()
        self.roads = dict()
        self.sections = dict()

        self.save_file_name = save_file_name
        if not os.path.exists(save_file_name):
            self.lane_edges = list()
            self.lane_center_lines = list()
        else:
            f = bz2.BZ2File(self.save_file_name, 'rb')
            self.lane_edges = pickle.load(f)
            self.lane_center_lines = pickle.load(f)
            f.close()

    def add_lane_edge(self, lane_edge):
        self.lane_edges.append(lane_edge)

    def add_center_line(self, center_line):
        self.lane_center_lines.append(center_line)

    def save(self):
        f = bz2.BZ2File(self.save_file_name, 'wb')
        pickle.dump(self.lane_edges, f)
        pickle.dump(self.lane_center_lines, f)
        f.close()
