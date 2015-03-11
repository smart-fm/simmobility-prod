//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "PT_NetworkEntities.hpp"


using namespace std;
using namespace sim_mob;

PT_NetworkEdges::PT_NetworkEdges():startStop(""),endStop(""),rType(""),road_index(""),roadEdgeId(""),
rServiceLines(""),linkTravelTimeSecs(0),edgeId(0),waitTimeSecs(0),walkTimeSecs(0),transitTimeSecs(0),
transferPenaltySecs(0),dayTransitTimeSecs(0),distKMs(0)
{}

PT_NetworkEdges::~PT_NetworkEdges() {}

PT_NetworkVertices::PT_NetworkVertices():stopId(""),stopCode(""),stopName(""),stopLatitude(0),
		stopLongitude(0),ezlinkName(""),stopType(0),stopDesc("")
{}

PT_NetworkVertices::~PT_NetworkVertices() {}
