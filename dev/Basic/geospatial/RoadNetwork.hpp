/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <vector>


namespace sim_mob
{

//Forward declarations
class Node;


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
 *
 * \note
 * This is a skeleton class. All functions are defined in this header file.
 * When this class's full functionality is added, these header-defined functions should
 * be moved into a separate cpp file.
 */
class RoadNetwork {
public:
	DRIVING_SIDE drivingSide;

	const std::vector<sim_mob::Node*>& getNodes() { return nodes; }

private:
	//Temp: All nodes in a network.
	std::vector<sim_mob::Node*> nodes;


friend class sim_mob::aimsun::Loader;

};





}
