#!/usr/bin/python

# Copyright Singapore-MIT Alliance for Research and Technology

from PyQt4 import QtGui, QtCore

class Lane_edge:
    def __init__(self, polyline, type, orig_edge=None):
        self.polyline = polyline
        self.type = type

        if orig_edge:
            self.section_id = orig_edge.section_id
            self.index = orig_edge.index
        else:
            self.section_id = 0
            self.index = 0

    def set_section_id(self, section_id):
        self.section_id = section_id

class Center_line:
    def __init__(self, polyline, orig_line=None):
        self.polyline = polyline
        self.type = 'c'
        if orig_line:
            self.section_id = orig_line.section_id
            self.index = orig_line.index  # This is the lane index in the road segment

            self.can_go_straight = orig_line.can_go_straight
            self.can_turn_left = orig_line.can_turn_left
            self.can_turn_right = orig_line.can_turn_right
            self.can_turn_on_red_signal = orig_line.can_turn_on_red_signal
            self.can_change_lane_left = orig_line.can_change_lane_left
            self.can_change_lane_right = orig_line.can_change_lane_right
            self.is_road_shoulder = orig_line.is_road_shoulder
            self.is_bicycle_lane = orig_line.is_bicycle_lane
            self.is_vehicle_lane = orig_line.is_vehicle_lane
            self.is_pedestrian_lane = orig_line.is_pedestrian_lane
            self.is_standard_bus_lane = orig_line.is_standard_bus_lane
            self.is_whole_day_bus_lane = orig_line.is_whole_day_bus_lane
            self.is_high_occupancy_vehicle_lane = orig_line.is_high_occupancy_vehicle_lane
            self.can_freely_park_here = orig_line.can_freely_park_here
            self.can_stop_here = orig_line.can_stop_here
            self.is_U_turn_allowed_here = orig_line.is_U_turn_allowed_here
        else:
            self.section_id = 0
            self.index = 0  # This is the lane index in the road segment

            self.can_go_straight = False
            self.can_turn_left = False
            self.can_turn_right = False
            self.can_turn_on_red_signal = False
            self.can_change_lane_left = False
            self.can_change_lane_right = False
            self.is_road_shoulder = False
            self.is_bicycle_lane = False
            self.is_vehicle_lane = False
            self.is_pedestrian_lane = False
            self.is_standard_bus_lane = False
            self.is_whole_day_bus_lane = False
            self.is_high_occupancy_vehicle_lane = False
            self.can_freely_park_here = False
            self.can_stop_here = False
            self.is_U_turn_allowed_here = False

    def set_section_id(self, section_id):
        self.section_id = section_id
