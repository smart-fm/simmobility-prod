#!/usr/bin/python

# Copyright Singapore-MIT Alliance for Research and Technology

from PyQt4 import QtGui, QtCore

class Lane_edge:
    def __init__(self, polyline, type):
        self.polyline = polyline
        self.type = type
        self.section_id = 0
        self.index = 0

    def set_section_id(self, section_id):
        self.section_id = section_id

class Center_line:
    def __init__(self, polyline):
        self.polyline = polyline
        self.type = 'c'
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
