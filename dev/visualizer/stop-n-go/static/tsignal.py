#!/usr/bin/env python

# Copyright Singapore-MIT Alliance for Research and Technology

import re, json
from error import Error

def json_hook(dictionary):
    node = dictionary["node"]
    va = dictionary["va"]
    vb = dictionary["vb"]
    vc = dictionary["vc"]
    vd = dictionary["vd"]
    pa = dictionary["pa"]
    pb = dictionary["pb"]
    pc = dictionary["pc"]
    pd = dictionary["pd"]
    aa = dictionary["aa"]
    ab = dictionary["ab"]
    ac = dictionary["ac"]
    ad = dictionary["ad"]
    return Traffic_signal(node, va, vb, vc, vd, pa, pb, pc, pd)

class Traffic_signal:
    regexp = re.compile('\("Signal-location", 0, (0x[0-9a-fA-F]+), (.+)\)')

    @staticmethod
    def parse(line, road_items):
        mo = Traffic_signal.regexp.search(line)
        if not mo:
            return False

        pointer = mo.group(1)
        if pointer in road_items:
            raise Error("traffic-signal: %s already exists in road_items" % pointer)
        data = mo.group(2)
        road_items[pointer] = json.loads(data, object_hook=json_hook)
        return True

    def __init__(self, node, va, vb, vc, vd, pa, pb, pc, pd):
        self.node = node
        self.va = va
        self.vb = vb
        self.vc = vc
        self.vd = vd
        self.pa = pa
        self.pb = pb
        self.pc = pc
        self.pd = pd

    def resolve(self, road_items):
        if self.node not in road_items:
            raise Error("traffic-signal: unknown node %s" % self.node)
        if self.va not in road_items:
            raise Error("traffic-signal: unknown va %s" % self.va)
        if self.vb not in road_items:
            raise Error("traffic-signal: unknown vb %s" % self.vb)
        if self.vc not in road_items:
            raise Error("traffic-signal: unknown vc %s" % self.vc)
        if self.vd not in road_items:
            raise Error("traffic-signal: unknown vd %s" % self.vd)
        if self.pa not in road_items:
            raise Error("traffic-signal: unknown pa %s" % self.pa)
        if self.pb not in road_items:
            raise Error("traffic-signal: unknown pb %s" % self.pb)
        if self.pc not in road_items:
            raise Error("traffic-signal: unknown pc %s" % self.pc)
        if self.pd not in road_items:
            raise Error("traffic-signal: unknown pd %s" % self.pd)
        self.node = road_items[self.node]
        self.va = road_items[self.va]
        self.vb = road_items[self.vb]
        self.vc = road_items[self.vc]
        self.vd = road_items[self.vd]

    def __repr__(self):
        return "traffic-signal(%s)" % self.node
