#!/usr/bin/env python

# Copyright Singapore-MIT Alliance for Research and Technology

import os, bz2, pickle

from database import Database

cache_file_name = "SimMob.cache"

def load_road_network(road_network):
    if os.path.exists(cache_file_name):
        print "loading SimMobility road network..."
        f = bz2.BZ2File(cache_file_name, 'rb')
        road_network.nodes = pickle.load(f)
        road_network.roads = pickle.load(f)
        road_network.sections = pickle.load(f)
        f.close()
        print "loading done."
    else:
        print "loading from SimMobility database..."
        database = Database()
        database.load(road_network)
        print "loading done."
        # If the road network is too large, then pickle will fail.  So we disable the caching.
        return

        print "caching SimMobility road network locally..."
        f = bz2.BZ2File(cache_file_name, 'wb')
        pickle.dump(road_network.nodes, f)
        pickle.dump(road_network.roads, f)
        pickle.dump(road_network.sections, f)
        f.close()
        print "caching done."
