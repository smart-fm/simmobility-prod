/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <vector>
#include <string>
#include <set>
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
	static std::string LoadNetwork(const std::string& connectionStr, std::map<std::string, std::string>& storedProcs, sim_mob::RoadNetwork& rn);

	//Semi-private functions
	static void ProcessGeneralNode(sim_mob::RoadNetwork& res, Node& src);
	static void ProcessUniNode(sim_mob::RoadNetwork& res, Node& src);
	static void ProcessSection(sim_mob::RoadNetwork& res, Section& src);
	static void ProcessTurning(sim_mob::RoadNetwork& res, Turning& src);
	static void ProcessSectionPolylines(sim_mob::RoadNetwork& res, Section& src);
	static void GenerateACrossing(sim_mob::RoadNetwork& resNW, Node& origin, Node& dest, std::vector<int>& laneIDs);

};



}
}
