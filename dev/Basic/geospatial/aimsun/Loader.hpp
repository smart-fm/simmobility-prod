/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <vector>
#include <map>
#include <iostream>


namespace sim_mob
{

//Forward declarations
class RoadNetwork;

namespace aimsun
{

//Forward declarations
class Node;
class Section;
class Turning;
class Polyline;



/**
 * Class for loading AimSun data and converting it to a format compatible with out network.
 */
class Loader {
public:
	///Load AIMSUN-generated data. Currently, this is done in three steps:
	/// 1) Load data into AIMSUN-style classes. (This will presumably be automated with
	///    some kind of persistence framework later.)
	/// 2) Transform these classes into the Sim Mobility format, logging any irregularities.
	/// 3) Discard the AIMSUN classes; return the Sim Mobility classes.
	///Returns false if an exception was thrown or if something else unexpected occurred
	//  (e.g., Node ID reference that doesn't exist).
	static bool LoadNetwork(sim_mob::RoadNetwork& rn);

private:

	///Private function for saving data. Required to take advantage of Loader's "friend" status with the
	///   RoadNetwork class.
	static void SaveSimMobilityNetwork(sim_mob::RoadNetwork& res, std::map<int, sim_mob::aimsun::Node>& nodes,
			std::map<int, sim_mob::aimsun::Section>& sections, std::map<int, sim_mob::aimsun::Turning>& turnings,
			std::multimap<int, sim_mob::aimsun::Polyline>& polylines);
};



}
}
