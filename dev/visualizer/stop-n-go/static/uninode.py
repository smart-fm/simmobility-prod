#!/usr/bin/env python

# Copyright Singapore-MIT Alliance for Research and Technology

import re, json
from error import Error
from point import Point

def json_hook(dictionary):
    x = int(dictionary["xPos"])
    y = int(dictionary["yPos"])
    db_id = int(dictionary["aimsun-id"])
    return Uni_node(x, y, db_id)

class Uni_node(Point):
    regexp = re.compile('\("uni-node", 0, (0x[0-9a-fA-F]+), {(.+),}\)')

    @staticmethod
    def parse(line, road_items):
        mo = Uni_node.regexp.search(line)
        if not mo:
            return False

        pointer = mo.group(1)
        if pointer in road_items:
            raise Error("uni-node: %s already exists in road_items" % pointer)
        data = '{' + mo.group(2) + '}' 
        road_items[pointer] = json.loads(data, object_hook=json_hook)
        return True

    def __init__(self, x, y, db_id):
        super(Uni_node, self).__init__(x, y)
        self.db_id = db_id

    def resolve(self, road_items):
        pass

    def __repr__(self):
        return "uni-node(%d, %d)" % (self.x, self.y)
