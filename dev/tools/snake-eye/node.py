#!/usr/bin/env python

# Copyright Singapore-MIT Alliance for Research and Technology

import re
from point import Point

class Node(Point):
    regexp = re.compile("aimsun::node id=(\d+), x=(\d+), y=(\d+), is-intersection=(\w+)")

    def __init__(self, x, y, is_intersection):
        super(Node, self).__init__(x, y)
        self.is_intersection = is_intersection

    def __repr__(self):
        node_type = "multi-node" if self.is_intersection else "uni-node"
        return "node(%d, %d, %s)" % (self.x, self.y, node_type)

    @staticmethod
    def check(line, things):
        mo = Node.regexp.search(line)
        if not mo:
            return False
        if mo.group(1) in things:
            raise "node %s already found" % mo.group(1)
        is_intersection = True if "true" == mo.group(4) else False
        things[mo.group(1)] = Node(int(mo.group(2)), int(mo.group(3)), is_intersection)
        return True
