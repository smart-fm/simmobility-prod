#!/usr/bin/env python

# Copyright Singapore-MIT Alliance for Research and Technology

import re, json
from error import Error

def json_hook(dictionary):
    road_name = dictionary["road-name"].title()
    start_node = dictionary["start-node"]
    end_node = dictionary["end-node"]
    forward_paths = get_paths(dictionary["fwd-path"])
    reverse_paths = get_paths(dictionary["rev-path"])
    return Link(road_name, start_node, end_node, forward_paths, reverse_paths)

def get_paths(string):
    if "[]" == string:
        return ""
    if string.endswith(",]"):
        return string[1:-2]

class Link:
    regexp = re.compile('\("link", 0, (0x[0-9a-fA-F]+), {(.+),}\)')

    @staticmethod
    def parse(line, road_items):
        mo = Link.regexp.search(line)
        if not mo:
            return False

        pointer = mo.group(1)
        if pointer in road_items:
            raise Error("link: %s already exists in road_items" % pointer)
        data = '{' + mo.group(2) + '}'
        road_items[pointer] = json.loads(data, object_hook=json_hook)
        return True

    def __init__(self, road_name, start_node, end_node, forward_paths, reverse_paths):
        self.road_name = road_name
        self.start_node = start_node
        self.end_node = end_node
        self.forward_paths = forward_paths
        self.reverse_paths = reverse_paths

    def resolve(self, road_items):
        if self.start_node not in road_items:
            raise Error("link: unknown start-node %s" % self.start_node)
        if self.end_node not in road_items:
            raise Error("link: unknown end-node %s" % self.end_node)
        self.start_node = road_items[self.start_node]
        self.end_node = road_items[self.end_node]
        self.forward_paths = resolve_paths(self.forward_paths, road_items, self.road_name)
        self.reverse_paths = resolve_paths(self.reverse_paths, road_items, self.road_name)

    def __repr__(self):
        return "link(from %s to %s)" % (self.start_node, self.end_node)

def resolve_paths(string, road_items, road_name):
    paths = list()
    for pointer in string.split(','):
        if len(pointer):
            if pointer not in road_items:
                raise Error("link: unknown road-segment %s" % pointer)
            road_segment = road_items[pointer]
            paths.append(road_segment)
            road_segment.set_road_name(road_name)
    return paths
