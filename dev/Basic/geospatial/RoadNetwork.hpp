/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <vector>


namespace sim_mob
{

//Forward declarations
class Node;
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

	///Retrieve list of all Nodes in this Road Network.
	///
	///\todo This needs to eventually have some structure; see the wiki for an example.
	const std::vector<sim_mob::Node*>& getNodes() { return nodes; }

private:
	std::vector<sim_mob::Node*> nodes;

	//Temporary: Geometry will eventually make specifying nodes easier.
	std::vector<sim_mob::Link*> links;


friend class sim_mob::aimsun::Loader;

};





}
