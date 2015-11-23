//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <vector>
#include <string>
#include <set>
#include <map>
#include <iostream>
#include <list>

#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include "soci/soci.h"
#include "soci/postgresql/soci-postgresql.h"
#include "path/Common.hpp"
#include "path/PathSetParam.hpp"

namespace sim_mob
{

//Forward declarations
class TripChainItem;
class RoadNetwork;
class RoadSegment;
class DynamicVector;
class Link;
class ProfileBuilder;
class SinglePath;
class PathSet;
class ERP_Gantry_Zone;
class ERP_Section;
class ERP_Surcharge;
class LinkTravelTime;
class PT_PathSet;
class TurningPath;
class TurningConflict;
class TurningPolyline;
class Polypoint;

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
	static void LoadNetwork(const std::string& connectionStr, const std::map<std::string, std::string>& storedProcs);
	static void loadSegNodeType(const std::string& connectionStr, const std::map<std::string, std::string>& storedProcs, sim_mob::RoadNetwork& rn);
	static bool excuString(soci::session& sql,std::string& str);
	///For partial network loading.
	static std::map<std::string, std::vector<sim_mob::TripChainItem*> > LoadTripChainsFromNetwork(const std::string& connectionStr, const std::map<std::string, std::string>& storedProcs);
	/// get CBD's border information
	static void getCBD_Border(
			std::set< std::pair<const sim_mob::RoadSegment*, const sim_mob::RoadSegment*> > &in,
			std::set< std::pair<const sim_mob::RoadSegment*, const sim_mob::RoadSegment*> > & out);

	static void getCBD_Nodes(std::map<unsigned int, const sim_mob::Node*>& nodes);

	///	get all CBD's segments
	static void getCBD_Segments(std::set<const sim_mob::RoadSegment*> & zoneSegments);

	// get the screen line segments
	static void getScreenLineSegments(const std::string& connectionStr,
                const std::map<std::string, std::string>& storedProcs, std::set<unsigned int>& screenLineList);

	//Semi-private functions
	static void ProcessGeneralNode(sim_mob::RoadNetwork& res, Node& src);
	static void ProcessUniNode(sim_mob::RoadNetwork& res, Node& src);
	static void ProcessSection(sim_mob::RoadNetwork& res, Section& src);
	static void ProcessTurning(sim_mob::RoadNetwork& res, Turning& src);
	static void ProcessSectionPolylines(sim_mob::RoadNetwork& res, Section& src);
	static void FixupLanesAndCrossings(sim_mob::RoadNetwork& res);

	//Ugh
	static void TMP_TrimAllLaneLines(sim_mob::RoadSegment* seg, const sim_mob::DynamicVector& cutLine, bool trimStart);

	/**
	 * Creates an intersection manager for every multi-node that doesn't have a traffic signal
	 * @param roadNetwork the road segment for which intersection managers must be created
	 */
	static void CreateIntersectionManagers(const sim_mob::RoadNetwork& roadNetwork);
};

}

}
