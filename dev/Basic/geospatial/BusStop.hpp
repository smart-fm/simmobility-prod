#pragma once

#include <vector>

#include "RoadItem.hpp"


namespace sim_mob
{

/**
 * Representation of a Bus Stop.
 *
 * \note
 * This is a skeleton class. All functions are defined in this header file.
 * When this class's full functionality is added, these header-defined functions should
 * be moved into a separate cpp file.
 */
class BusStop : public sim_mob::RoadItem {
public:
	///Which RoadItem and lane is this bus stop located at?
	std::pair<const RoadItem*, unsigned int> location;

	///Is this a bus bay, or does it take up space on the lane?
	///Bus bays are always to the dominant position away from the lane.
	///So, if drivingSide = OnLeft, then the bay extends to the left in its own lane.
	bool is_bay;

	///Is this a bus terminal? Currently, the only effect this has is to avoid
	///   requiring a bus to wait for the bus in front of it to leave.
	bool is_terminal;

	///How many meters of "bus" can park in the bus lane/bay to pick up pedestrians.
	///  Used to more easily represent double-length or mini-buses.
	unsigned int busCapacityAsLength;

	///Is the pedestrian waiting area sheltered? Currently does not affect anything.
	bool has_shelter;


private:
	///Get the bus lines available at this stop. Used for route planning.
	///NOTE: the void* is obviously temporary
	void* getBusLines() { return (void*)0; }

	///Get a list of bus arrival times. Pedestrians can consult this (assuming the bus stop is VMS-enabled).
	///NOTE: the void* is obviously temporary
	void* getBusArrivalVMS() { return (void*)0; }


};





}
