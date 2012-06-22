#!/usr/bin/env python

# Copyright Singapore-MIT Alliance for Research and Technology

import re, json

def json_hook(dictionary):
    va = get_vehicle_colors(dictionary["va"])
    vb = get_vehicle_colors(dictionary["vb"])
    vc = get_vehicle_colors(dictionary["vc"])
    vd = get_vehicle_colors(dictionary["vd"])
    pa = get_pedestrian_color(dictionary["pa"])
    pb = get_pedestrian_color(dictionary["pb"])
    pc = get_pedestrian_color(dictionary["pc"])
    pd = get_pedestrian_color(dictionary["pd"])

def get_vehicle_colors(string):
    left, straight, right = string.spilt(',')
    left = get_traffic_color(left)
    straight = get_traffic_color(straight)
    right = get_traffic_color(right)

def get_pedestrian_color(string):
    return get_traffic_color(int(string))

def get_traffic_color(number):
    if number == 1: return Traffic_Signal.Red
    if number == 2: return Traffic_Signal.Amber
    if number == 3: return Traffic_Signal.Green

class Traffic_signal:
    regexp = re.compile('\("Signal",(\d+),(0x[0-9a-fA-F]+),(.+)\)')
    Red, Amber, Green = (1, 2, 3)

    @staticmethod
    def parse(line, agents, animator):
        mo = Traffic_signal.regexp.search(line)
        if not mo:
            return False

        frame_number = int(mo.group(1))
        pointer = mo.group(2)
        if pointer not in agents:
            agents[pointer] = Traffic_signal()

    def __init__(self):
        pass

    def __repr__(self):
        return "traffic-signal"
