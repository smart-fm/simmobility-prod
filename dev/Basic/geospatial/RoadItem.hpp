#pragma once

//#include "Node.hpp"


namespace sim_mob
{


//Forward declarations
class Node;


/**
 * Base class for geospatial items which take up physical space. (Contrast with Nodes, which
 * only take up abstract space.) Each RoadItem has a "start" and "end" Node, which place it
 * within a 2-D coordinate system. Additional data may be used to fine-tune this item's shape
 * in 2-D space, or even to extend it into 3-D space.
 *
 * \note
 * This is a skeleton class. All functions are defined in this header file.
 * When this class's full functionality is added, these header-defined functions should
 * be moved into a separate cpp file.
 */
class RoadItem {
public:


protected:
	const sim_mob::Node* start;
	const sim_mob::Node* end;



};





}
