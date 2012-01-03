#!/usr/bin/env python

# Copyright Singapore-MIT Alliance for Research and Technology

import psycopg2
from node import Node
from road import Road
from section import Section
from error import Error

def convert_meters_to_centimeters(x, y):
    return int(x * 100), int(y * 100)

class Database:
    def __init__(self):
        self.host = "172.18.127.157"
        self.port = "5432"
        self.user = "postgres"
        self.password = "S!Mm0bility"
        self.database_name = "SimMobility_DB"

    def open(self):
        self.connection = psycopg2.connect(host=self.host, port=self.port,
                                           user=self.user, password=self.password,
                                           database=self.database_name)
        self.cursor = self.connection.cursor()

    def close(self):
        self.cursor.close()
        self.connection.close()

    def load(self, road_network):
        self.open()
        self.load_nodes(road_network)
        self.load_sections(road_network)
        self.load_lane_markings(road_network)
        self.load_kerb_lines(road_network)
        self.close()

        sort_polylines(road_network.sections)

    def load_nodes(self, road_network):
        self.cursor.execute('SELECT * FROM get_node();')
        for id, x, y, is_intersection in self.cursor.fetchall():
            if id in road_network.nodes:
                raise Error("node id=%d already exists" % id)
            x, y = convert_meters_to_centimeters(x, y)
            road_network.nodes[id] = Node(id, x, y, is_intersection)

    def load_sections(self, road_network):
        self.cursor.execute('SELECT * FROM get_section();')
        for (id, name, number_lanes, speed_limit, capacity,
             from_node_id, to_node_id, length) in self.cursor.fetchall():

            if id in road_network.sections:
                raise Error("section id=%d already exists" % id)
            if from_node_id not in road_network.nodes:
                raise Error("section id=%d unknown from-node=%d" % (id, from_node_id))
            if to_node_id not in road_network.nodes:
                raise Error("section id=%d unknown to-node=%d" % (id, to_node_id))

            road_name = name.title()
            if road_name not in road_network.roads:
                road_network.roads[road_name] = Road(road_name)
            road = road_network.roads[road_name]

            from_node = road_network.nodes[from_node_id]
            to_node = road_network.nodes[to_node_id]
            section = Section(id, road, number_lanes, speed_limit,
                              capacity, from_node, to_node, length)
            road_network.sections[id] = section
            road.sections.append(section)

    def load_lane_markings(self, road_network):
        self.cursor.execute('SELECT * FROM get_lanes();')
        for id, type, type_desc, section_id, road_name, x, y, row_number in self.cursor.fetchall():
            if section_id not in road_network.sections:
                raise Error("lane id=%d unknown-section=%d" % (id, section_id))
            section = road_network.sections[section_id]
            x, y = convert_meters_to_centimeters(x, y)
            section.add_lane_marking_point(id, type, x, y, row_number)

    def load_kerb_lines(self, road_network):
        self.cursor.execute('SELECT * FROM get_kerblines();')
        for id, lane_id, x, y, row_number, type, section_id in self.cursor.fetchall():
            if section_id not in road_network.sections:
                raise Error("kerb id=%d unknown-section=%d" % (id, section_id))
            section = road_network.sections[section_id]
            x, y = convert_meters_to_centimeters(x, y)
            section.add_kerb_line_point(id, lane_id, type, x, y, row_number)

def sort_polylines(sections):
    for section in sections.values():
        for lane_marking in section.lane_markings.values():
            lane_marking.sort_polyline()
        for kerb_line in section.kerb_lines.values():
            kerb_line.sort_polyline()
