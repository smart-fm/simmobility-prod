#!/usr/bin/env python

# Copyright Singapore-MIT Alliance for Research and Technology

import re, json
from error import Error
from point import Point

def json_hook(dictionary):
    x = int(dictionary["xPos"])
    y = int(dictionary["yPos"])
    db_id = int(dictionary["aimsun-id"])
    return Multi_node(x, y, db_id)

class Multi_node(Point):
    regexp = re.compile('\("multi-node", 0, (0x[0-9a-fA-F]+), {(.+),}\)')

    @staticmethod
    def parse(line, road_items):
        mo = Multi_node.regexp.search(line)
        if not mo:
            return False

        pointer = mo.group(1)
        if pointer in road_items:
            raise Error("multi-node: %s already exists in road_items" % pointer)
        data = '{' + mo.group(2) + '}' 
        road_items[pointer] = json.loads(data, object_hook=json_hook)
        return True

    def __init__(self, x, y, db_id):
        super(Multi_node, self).__init__(x, y)
        self.db_id = db_id

    def resolve(self, road_items):
        pass

    def __repr__(self):
        return "multi-node(%d, %d)" % (self.x, self.y)
