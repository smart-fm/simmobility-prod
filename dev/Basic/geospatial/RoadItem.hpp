/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include "../constants.h"


namespace sim_mob
{


//Forward declarations
class Node;


/**
 * Base class for geospatial items which take up physical space. (Contrast with Nodes, which
 * only take up abstract space.) Each RoadItem has a "start" and "end" Node, which place it
 * within a 2-D coordinate system. Additional data may be used to fine-tune this item's shape
 * in 2-D space, or even to extend it into 3-D space.
 */
class RoadItem {
public:
	RoadItem(sim_mob::Node* start=nullptr, sim_mob::Node* end=nullptr) : start(start), end(end) {}

	const sim_mob::Node* getStart() { return start; }
	const sim_mob::Node* getEnd() { return end; }


protected:
	sim_mob::Node* start;
	sim_mob::Node* end;



};





}
