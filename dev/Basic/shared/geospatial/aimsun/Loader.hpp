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
#include "soci.h"
#include "soci-postgresql.h"

#include "path/Common.hpp"

namespace sim_mob
{

//Forward declarations
class TripChainItem;
class RoadNetwork;
class RoadSegment;
class DynamicVector;
class Link;
class ProfileBuilder;
class Conflux;
class SegmentStats;
class SinglePath;
class PathSet;
class ERP_Gantry_Zone;
class ERP_Section;
class ERP_Surcharge;
class LinkTravelTime;


enum HasPath
{
	PSM_HASPATH,//found and valid
	PSM_NOGOODPATH,//previous attempt to build pathset failed
	PSM_NOTFOUND,//search didn't find anything
	PSM_UNKNOWN,
};

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
	static void LoadNetwork(const std::string& connectionStr, const std::map<std::string, std::string>& storedProcs, sim_mob::RoadNetwork& rn, std::map<std::string, std::vector<sim_mob::TripChainItem*> >& tcs, ProfileBuilder* prof);
    static void loadSegNodeType(const std::string& connectionStr, const std::map<std::string, std::string>& storedProcs, sim_mob::RoadNetwork& rn);
	static bool LoadPathSetDataWithId(const std::string& connectionStr,
			std::map<std::string,sim_mob::SinglePath*>& pathPool,
			std::map<std::string,SinglePath*> &waypoint_singlepathPool,
			std::map<std::string,boost::shared_ptr<sim_mob::PathSet> > & pathSetPool,std::string& pathset_id);
	static sim_mob::HasPath loadSinglePathFromDB(soci::session& sql,std::string& pathset_id,
			std::set<sim_mob::SinglePath*, sim_mob::SinglePath>& spPool,const std::string functionName,
//			std::stringstream *outDbg=0,
			const std::set<const sim_mob::RoadSegment *> & excludedRS = std::set<const sim_mob::RoadSegment *>());
	static void LoadERPData(const std::string& connectionStr,
			std::map<std::string,std::vector<sim_mob::ERP_Surcharge*> > &erp_surcharge_pool,
			std::map<std::string,sim_mob::ERP_Gantry_Zone*>& erp_gantry_zone_pool,
			std::map<int,sim_mob::ERP_Section*>& erp_section_pool);
	static bool createTable(soci::session& sql,std::string& table_name);
	static bool insertData2TravelTimeTmpTable(const std::string& connectionStr,	std::string& table_name, sim_mob::LinkTravelTime& data);
	static bool upsertTravelTime(soci::session& sql,const std::string& csvFileName, const std::string& tableName);
	static bool insertCSV2Table(soci::session& sql, std::string& table_name,const std::string& csvFileName);
	static bool truncateTable(soci::session& sql, std::string& table_name);
	static bool excuString(soci::session& sql,std::string& str);
	static void LoadDefaultTravelTimeData(soci::session& sql, std::map<unsigned long,std::vector<sim_mob::LinkTravelTime> >& pool);
	static bool LoadRealTimeTravelTimeData(soci::session& sql,int interval, sim_mob::AverageTravelTime& pool);
	static bool storeSinglePath(soci::session& sql,
					std::set<sim_mob::SinglePath*,sim_mob::SinglePath>& pathPool,const std::string pathSetTableName);
	///For partial network loading.
	static std::map<std::string, std::vector<sim_mob::TripChainItem*> > LoadTripChainsFromNetwork(const std::string& connectionStr, const std::map<std::string, std::string>& storedProcs);
	/// get CBD's border information
	static void getCBD_Border(
			std::set< std::pair<const sim_mob::RoadSegment*, const sim_mob::RoadSegment*> > &in,
			std::set< std::pair<const sim_mob::RoadSegment*, const sim_mob::RoadSegment*> > & out);
	///	get all CBD's segments
	static void getCBD_Segments(std::set<const sim_mob::RoadSegment*> & zoneSegments);

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
	 * constructs confluxes around each multinode
	 * @param rdnw the road network
	 */
	static void ProcessConfluxes(const sim_mob::RoadNetwork& rdnw);

	/**
	 * creates a list of SegmentStats for a given segment depending on the stops
	 * in the segment. The list splitSegmentStats will contain SegmentStats objects
	 * containing bus stops (and quite possibly a last SegmentStats with no bus stop)
	 * @param rdSeg the road segment for which stats must be created
	 * @param splitSegmentStats vector of SegmentStats* to be filled up
	 */
	static void CreateSegmentStats(const sim_mob::RoadSegment* rdSeg, std::list<sim_mob::SegmentStats*>& splitSegmentStats);
};

}

/**
 * Class for find the bus line from source destination nodes.
 * \author meenu
 * \author zhang huai peng
  */
class BusStop;
class Busline;
class Node;
class RoadSegment;
class BusStopFinder
{
public:
	BusStopFinder(const Node* src, const Node* dest);
	Busline* getBusLineToTake(){ return BusLineToTake; }
	BusStop* getSourceBusStop(){ return originBusStop; }
	BusStop* getDestinationBusStop(){ return destBusStop;}

private:
	BusStop* findNearbyBusStop(const Node* src);
	//Busline* findBusLineToTaken();
	BusStop* getBusStop(const Node* node,sim_mob::RoadSegment* segment);

private:
	BusStop* originBusStop;
    BusStop* destBusStop;
    Busline* BusLineToTake;
};

}
