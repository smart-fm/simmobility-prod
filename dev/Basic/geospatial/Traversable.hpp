/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include "../constants.h"


namespace sim_mob
{


//Forward declarations
class Node;


/**
 * Base class for geospatial items which take up physical space and can be traversed.
 * (Contrast: Nodes, which can be traversed but don't take up physical space.)
 * (Compare: RoadItems which take up physical space but can't be traversed.)
 *
 * Each RoadItem has a "start" and "end" Node, which place it within a 2-D coordinate system.
 * Additional data may be used to fine-tune this item's shape in 2-D space, or even to extend
 * it into 3-D space.
 */
class Traversable {
public:
	Traversable() : start(nullptr), end(nullptr) {}

	//NOTE: Shouldn't these return const Node* const ?
	const sim_mob::Node* getStart() const { return start; }
	const sim_mob::Node* getEnd() const { return end; }


protected:
	sim_mob::Node* start;
	sim_mob::Node* end;



};





}
