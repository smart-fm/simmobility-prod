/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <vector>


namespace sim_mob
{

//Forward declarations
class MultiNode;
class Link;


namespace aimsun
{
//Forward declaration
class Loader;
}


/**
 * The side of the road on which cars drive. For the USA, this is DRIVES_ON_RIGHT;
 * for Singapore it is DRIVES_ON_LEFT. This affects the context of "can_turn_right_on_red",
 * and may affect lane merging rules.
 */
enum DRIVING_SIDE {
	DRIVES_ON_LEFT,
	DRIVES_ON_RIGHT
};


/**
 * The main Road Network. (Currently, this class only contains the "drivingSide" variable)
 */
class RoadNetwork {
public:
	DRIVING_SIDE drivingSide;

	///Retrieve list of all MultiNodes (intersections & roundabouts) in this Road Network.
	///
	///\todo This needs to eventually have some structure; see the wiki for an example.
	const std::vector<sim_mob::MultiNode*>& getNodes() { return nodes; }

	///Retrieve a list of all Links (high-level paths between MultiNodes) in this Road Network.
	const std::vector<sim_mob::Link*>& getLinks() { return links; }

private:
	//Temporary: Geometry will eventually make specifying nodes and linkseasier.
	std::vector<sim_mob::MultiNode*> nodes;
	std::vector<sim_mob::Link*> links;


friend class sim_mob::aimsun::Loader;

};





}
