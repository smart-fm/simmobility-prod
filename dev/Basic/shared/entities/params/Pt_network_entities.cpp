//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "Pt_network_entities.hpp"


using namespace std;
using namespace sim_mob;

PT_NetworkEdges::PT_NetworkEdges():startStop(""),endStop(""),rType(""),road_index(""),roadEdgeId(""),
rServiceLines(""),linkTravelTime(0),edgeId(0),waitTime(0),walkTime(0),transitTime(0),
transferPenalty(0),dayTransitTime(0),dist(0)
{}

PT_NetworkEdges::~PT_NetworkEdges() {}

PT_NetworkVertices::PT_NetworkVertices():stopId(""),stopCode(""),stopName(""),stopLatitude(0),
		stopLongitude(0),ezlinkName(""),stopType(0),stopDesc("")
{}

PT_NetworkVertices::~PT_NetworkVertices() {}
