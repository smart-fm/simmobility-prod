#!/usr/bin/env python

# Copyright Singapore-MIT Alliance for Research and Technology

import re, json

class Session:
    def __init__(self):
        self.frame_interval_regexp = re.compile('Base Granularity: (\d+) ms')
        self.total_run_time_regexp = re.compile('Total Runtime: (\d+) ticks')
        self.warm_up_time_regexp = re.compile('Total Warmup: (\d+) ticks')
        self.agent_frame_rate_regexp = re.compile('Agent Granularity: (\d+) ticks')
        self.traffic_signal_frame_rate_regexp = re.compile('Signal Granularity: (\d+) ticks')
        self.path_choice_frame_rate_regexp = re.compile('Paths Granularity: (\d+) ticks')
        self.network_decomposition_frame_rate_regexp = re.compile('Decomp Granularity: (\d+) ticks')
        self.start_time_regexp = re.compile('Start time: (\d+):(\d+):(\d+)')

    def parse(self, line):
        if self.parse_for_frame_interval(line):
            return True
        if self.parse_for_total_run_time(line):
            return True
        if self.parse_for_warm_up_time(line):
            return True
        if self.parse_for_agent_frame_rate(line):
            return True
        if self.parse_for_traffic_signal_frame_rate(line):
            return True
        if self.parse_for_path_choice_frame_rate(line):
            return True
        if self.parse_for_network_decomposition_frame_rate(line):
            return True
        if self.parse_for_start_time(line):
            return True
        return False

    def parse_for_frame_interval(self, line):
        mo = self.frame_interval_regexp.search(line)
        if not mo:
            return False
        self.frame_interval = int(mo.group(1))
        return True

    def parse_for_total_run_time(self, line):
        mo = self.total_run_time_regexp.search(line)
        if not mo:
            return False
        self.total_run_time = int(mo.group(1))
        return True

    def parse_for_warm_up_time(self, line):
        mo = self.warm_up_time_regexp.search(line)
        if not mo:
            return False
        self.warm_up_time = int(mo.group(1))
        return True

    def parse_for_agent_frame_rate(self, line):
        mo = self.agent_frame_rate_regexp.search(line)
        if not mo:
            return False
        self.agent_frame_rate = int(mo.group(1))
        return True

    def parse_for_traffic_signal_frame_rate(self, line):
        mo = self.traffic_signal_frame_rate_regexp.search(line)
        if not mo:
            return False
        self.traffic_signal_frame_rate = int(mo.group(1))
        return True

    def parse_for_path_choice_frame_rate(self, line):
        mo = self.path_choice_frame_rate_regexp.search(line)
        if not mo:
            return False
        self.path_choice_frame_rate = int(mo.group(1))
        return True

    def parse_for_network_decomposition_frame_rate(self, line):
        mo = self.network_decomposition_frame_rate_regexp.search(line)
        if not mo:
            return False
        self.network_decomposition_frame_rate = int(mo.group(1))
        return True

    def parse_for_start_time(self, line):
        mo = self.start_time_regexp.search(line)
        if not mo:
            return False
        self.start_hour = int(mo.group(1))
        self.start_minute = int(mo.group(2))
        self.start_second = int(mo.group(3))
        return True
