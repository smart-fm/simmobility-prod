#pragma once

namespace sim_mob
{

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

private:



};





}
