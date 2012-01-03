#!/usr/bin/env python

# Copyright Singapore-MIT Alliance for Research and Technology

class Node:
    def __init__(self, id, x, y, is_intersection):
        self.id = id
        self.x = x
        self.y = y
        self.is_intersection = is_intersection

    def add_from_section(self, section):
        self.from_sections.append(section)

    def add_to_section(self, section):
        self.to_sections.append(section)
