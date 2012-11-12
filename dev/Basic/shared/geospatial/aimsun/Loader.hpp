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
class TripChainItem;
class RoadNetwork;
class RoadSegment;
class DynamicVector;
class Link;
class ProfileBuilder;


namespace aimsun
{

//Forward declarations
class Node;
class Section;
class Turning;
class Polyline;



/**
 * Class for loading AimSun data and converting it to a format compatible with out network.
 * \author Seth N. Hetu
 * \author LIM Fung Chai
 * \author Matthew Bremer Bruchon
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
	static std::string LoadNetwork(const std::string& connectionStr, const std::map<std::string, std::string>& storedProcs, sim_mob::RoadNetwork& rn, std::map<unsigned int, std::vector<sim_mob::TripChainItem*> >& tcs, ProfileBuilder* prof);

	//Semi-private functions
	static void ProcessGeneralNode(sim_mob::RoadNetwork& res, Node& src);
	static void ProcessUniNode(sim_mob::RoadNetwork& res, Node& src);
	static void ProcessSection(sim_mob::RoadNetwork& res, Section& src);
	static void ProcessTurning(sim_mob::RoadNetwork& res, Turning& src);
	static void ProcessSectionPolylines(sim_mob::RoadNetwork& res, Section& src);
	static void FixupLanesAndCrossings(sim_mob::RoadNetwork& res);

	//Ugh
	static void TMP_TrimAllLaneLines(sim_mob::RoadSegment* seg, const sim_mob::DynamicVector& cutLine, bool trimStart);
};



}
}
