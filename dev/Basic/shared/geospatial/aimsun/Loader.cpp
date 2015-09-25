//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "Loader.hpp"

#include <algorithm>
#include <boost/foreach.hpp>
#include <cmath>
#include <iostream>
#include <map>
#include <set>
#include <stdexcept>
#include <vector>

//NOTE: CMake should put the correct -I flags in for SOCI; be aware that some distros hide it though.
//#include <soci.h>
//#include <soci-postgresql.h>

#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "conf/settings/DisableMPI.h"
#include "entities/PersonLoader.hpp"
#include "entities/AuraManager.hpp"
#include "entities/conflux/SegmentStats.hpp"
#include "entities/misc/BusTrip.hpp"
#include "entities/vehicle/VehicleBase.hpp"
#include "geospatial/Point2D.hpp"
#include "geospatial/Node.hpp"
#include "geospatial/UniNode.hpp"
#include "geospatial/MultiNode.hpp"
#include "geospatial/Intersection.hpp"
#include "geospatial/Link.hpp"
#include "geospatial/RoadItem.hpp"
#include "geospatial/RoadSegment.hpp"
#include "geospatial/LaneConnector.hpp"
#include "geospatial/RoadNetwork.hpp"
#include "geospatial/Crossing.hpp"
#include "geospatial/Lane.hpp"
#include "geospatial/BusStop.hpp"
#include "geospatial/streetdir/StreetDirectory.hpp"
#include "geospatial/aimsun/CrossingLoader.hpp"
#include "geospatial/aimsun/LaneLoader.hpp"
#include "geospatial/aimsun/SOCI_Converters.hpp"
#include "geospatial/TurningSection.hpp"
#include "geospatial/TurningConflict.hpp"
#include "geospatial/TurningPolyline.hpp"
#include "geospatial/Polypoint.hpp"
#include "path/PathSetManager.hpp"
#include "path/PT_RouteChoiceLuaModel.hpp"

#include "logging/Log.hpp"
#include "metrics/Length.hpp"
#include "util/OutputUtil.hpp"
#include "util/GeomHelpers.hpp"
#include "util/DynamicVector.hpp"
#include "util/DailyTime.hpp"
#include "util/GeomHelpers.hpp"

//todo: almost all the following are already included in the above include-SOCI_Converters.hpp -->vahid
#include "BusStop.hpp"
#include "Node.hpp"
#include "Section.hpp"
#include "Crossing.hpp"
#include "Turning.hpp"
#include "Polyline.hpp"
#include "./Signal.hpp" //just a precaution
#include "Phase.hpp"

//Note: These will eventually have to be put into a separate Loader for non-AIMSUN data.
// fclim: I plan to move $topdir/geospatial/aimsun/* and entities/misc/aimsun/* to
// $topdir/database/ and rename the aimsun namespace to "database".
#include "entities/misc/TripChain.hpp"
#include "entities/misc/PublicTransit.hpp"
#include "entities/misc/aimsun/TripChain.hpp"
#include "entities/misc/aimsun/SOCI_Converters.hpp"
#include "entities/profile/ProfileBuilder.hpp"
#include "entities/conflux/Conflux.hpp"
#include "entities/Person.hpp"
#include "entities/signal/Signal.hpp"

#include "partitions/PartitionManager.hpp"
#include "partitions/BoundarySegment.hpp"
#include "entities/IntersectionManager.hpp"

using namespace sim_mob::aimsun;
using sim_mob::DynamicVector;
using sim_mob::Point2D;
using std::vector;
using std::string;
using std::set;
using std::map;
using std::pair;
using std::multimap;

namespace{
sim_mob::BasicLogger & pathsetLogger = sim_mob::Logger::log("pathset.log");
}
namespace {
const double SHORT_SEGMENT_LENGTH_LIMIT = 5 * sim_mob::PASSENGER_CAR_UNIT; // 5 times a car's length
const double BUS_LENGTH = 3 * sim_mob::PASSENGER_CAR_UNIT;
const std::string HIGHWAY_SERVICE_CATEGORY_STRING = "A"; //Category A segments are highway segments

class DatabaseLoader : private boost::noncopyable {
public:
	explicit DatabaseLoader(string const & connectionString);

	void LoadBasicAimsunObjects(map<string, string> const & storedProcedures);

	/**
	 * data to be loaded if we are running short-term
	 * @param storedProcs get db procedure name
	 */
	void LoadObjectsForShortTerm(map<string, string> const & storedProcs);
	/**
	 *  /brief load segment type, node type
	 *  /param storedProcs get db procedure name
	 *  /param rn road network object
	 */
	void loadObjectType(map<string, string> const & storedProcs,sim_mob::RoadNetwork& rn);
	void LoadTurningSection(map<string, string> const & storedProcs,sim_mob::RoadNetwork& rn);
	void storeTurningPoints(sim_mob::RoadNetwork& rn);
	void LoadERP_Surcharge(std::map<std::string,std::vector<sim_mob::ERP_Surcharge*> >& pool);
	void LoadERP_Section(std::map<int,sim_mob::ERP_Section*>& ERP_SectionPool);
	void LoadERP_Gantry_Zone(std::map<std::string,sim_mob::ERP_Gantry_Zone*>& ERP_GantryZonePool);
	static void loadLinkDefaultTravelTime(soci::session& sql,std::map<unsigned long,std::vector<sim_mob::LinkTravelTime> >& pool);
	static bool loadLinkRealTimeTravelTime(soci::session& sql,int interval, sim_mob::AverageTravelTime& pool);
	static bool CreateTable(soci::session& sql,std::string& tableName);
	bool InsertData2TravelTimeTmpTable(std::string& tableName,sim_mob::LinkTravelTime& data);
	static bool InsertCSV2Table(soci::session& sql,std::string& tableName,const std::string& csvFileName);
	static bool upsertTravelTime(soci::session& sql,const std::string& csvFileName, const std::string& tableName, double alpha);
	static bool TruncateTable(soci::session& sql,std::string& tableName);
	static bool ExcuString(soci::session& sql,std::string& str);
	// save path set data
	static bool InsertSinglePath2DB(soci::session& sql,std::set<sim_mob::SinglePath*,sim_mob::SinglePath>& spPool,const std::string pathSetTableName);
	static sim_mob::HasPath loadSinglePathFromDB(soci::session& sql,
					std::string& pathset_id,std::set<sim_mob::SinglePath*, sim_mob::SinglePath>& spPool,
					const std::string functionName,
					const std::set<const sim_mob::RoadSegment*>& excludedRS = std::set<const sim_mob::RoadSegment*>());
	static void loadPT_ChoiceSetFrmDB(soci::session& sql, std::string& pathSetId, sim_mob::PT_PathSet& pathSet);
	static void LoadPT_PathsetFrmDB(soci::session& sql, const std::string& funcName, int originalNode, int destNode, sim_mob::PT_PathSet& pathSet);

	void LoadScreenLineSegmentIDs(const map<string, string>& storedProcs, std::vector<unsigned long>& screenLines);
#ifndef SIMMOB_DISABLE_MPI
	void TransferBoundaryRoadSegment();
#endif

	void DecorateAndTranslateObjects();
	void PostProcessNetwork();

	//NOTE: Some of these are handled automatically under SaveSimMobilityNetwork, but should really be separated out.
	//      I've migrated saveTripChains into its own function in order to help with this.
	void SaveSimMobilityNetwork(sim_mob::RoadNetwork& res, std::map<std::string, std::vector<sim_mob::TripChainItem*> >& tcs);
	void saveTripChains(std::map<std::string, std::vector<sim_mob::TripChainItem*> >& tcs);


	map<int, Section> const & sections() const { return sections_; }
	const map<std::string, vector<const sim_mob::BusStop*> >& getRoute_BusStops() const { return route_BusStops; }
	const map<std::string, vector<const sim_mob::RoadSegment*> >& getRoute_RoadSegments() const { return route_RoadSegments; }

	static void getCBD_Border(const string & cnn,
			std::set< std::pair<const sim_mob::RoadSegment*, const sim_mob::RoadSegment*> > &in,
			std::set< std::pair<const sim_mob::RoadSegment*, const sim_mob::RoadSegment*> > & out);
	static void getCBD_Segments(const string & cnn,std::set<const sim_mob::RoadSegment*> & zoneSegments);

	static void getCBD_Nodes(const string& cnn, std::map<unsigned int, const sim_mob::Node*>& nodes);

private:
	soci::session sql_;


	map<int, Node> nodes_;
	map<int, Section> sections_;
	vector<Crossing> crossings_;
	vector<Lane> lanes_;
	map<int, Turning> turnings_;
	multimap<int, Polyline> polylines_;
	vector<TripChainItem> tripchains_;
	map<int, Signal> signals_;
	//vector<sim_mob::BusSchedule> busschedule_;

	map<std::string,BusStop> busstop_;
	map<std::string,BusStopSG> bustopSG_;
	multimap<int,Phase> phases_;//one node_id is mapped to many phases

	vector<sim_mob::BoundarySegment*> boundary_segments;

	map<std::string, vector<const sim_mob::BusStop*> > route_BusStops;
	map<std::string, vector<const sim_mob::RoadSegment*> > route_RoadSegments;

	map<Section*, sim_mob::RoadSegment*> sec_seg_;
private:
	void LoadNodes(const std::string& storedProc);
	void LoadSections(const std::string& storedProc);
	void LoadCrossings(const std::string& storedProc);
	void LoadLanes(const std::string& storedProc);
	void LoadTurnings(const std::string& storedProc);
	void LoadPolylines(const std::string& storedProc);
	void LoadTrafficSignals(const std::string& storedProc);

	void loadSegmentTypeTable(const std::string& storedProc,std::map<string,int>& segTypeMap);
	void loadNodeTypeTable(const std::string& storedProc,std::map<string,int>& nodeTypeMap);

	void loadTurningSectionTable(const std::string& storedProc, sim_mob::RoadNetwork& rn);
	void loadTurningConflictTable(const std::string& storedProc, sim_mob::RoadNetwork& rn);
	void loadTurningPolyline(const std::string& storedProc, const std::string& pointsStoredProc, sim_mob::RoadNetwork& rn);
	void loadPolypointByPolyline(const std::string& storedProc, sim_mob::TurningPolyline *t);
public:
	void LoadTripchains(const std::string& storedProc);

public:
	//New-style Loader functions can simply load data directly into the result vectors.
	void LoadPTBusDispatchFreq(const std::string& storedProc, std::vector<sim_mob::PT_BusDispatchFreq>& ptBusDispatchFreq);
	void LoadPTBusRoutes(const std::string& storedProc, std::vector<sim_mob::PT_BusRoutes>& ptBusRoutes, std::map<std::string, std::vector<const sim_mob::RoadSegment*> >& routeID_roadSegments);
	void LoadPTBusStops(const std::string& storedProc, std::vector<sim_mob::PT_BusStops>& ptBusStops,
			std::map<std::string, std::vector<const sim_mob::BusStop*> >& routeID_busStops,
			std::map<std::string, std::vector<const sim_mob::RoadSegment*> >& routeID_roadSegments);

	void LoadOD_Trips(const std::string& storedProc, std::vector<sim_mob::OD_Trip>& OD_Trips);

private:
	void LoadBusStop(const std::string& storedProc);
	void LoadBusStopSG(const std::string& storedProc);
	void LoadPhase(const std::string& storedProc);


#ifndef SIMMOB_DISABLE_MPI
	void LoadBoundarySegments();
#endif

	void createSignals();
    void createPlans(sim_mob::Signal_SCATS & signal);
    void createPhases(sim_mob::Signal_SCATS & signal);
    void createBusStopAgents();
};

DatabaseLoader::DatabaseLoader(string const & connectionString)
: sql_(soci::postgresql, connectionString)
{
}

//Sorting function for polylines
bool polyline_sorter (const Polyline* const p1, const Polyline* const p2)
{
	return p1->distanceFromSrc < p2->distanceFromSrc;
}

void DatabaseLoader::getCBD_Border(const string & cnn,
		std::set<std::pair<const sim_mob::RoadSegment*,const sim_mob::RoadSegment*> > &in,
		std::set<std::pair<const sim_mob::RoadSegment*,const sim_mob::RoadSegment*> > &out)
{
	soci::session sql(soci::postgresql, cnn);

	const std::string& inTurningFunc = sim_mob::ConfigManager::GetInstance().FullConfig().getDatabaseProcMappings().
														procedureMappings["restricted_reg_in_turning"];

	soci::rowset<sim_mob::CBD_Pair> rsIn = sql.prepare << std::string("select * from ") + inTurningFunc;
	for (soci::rowset<sim_mob::CBD_Pair>::iterator it = rsIn.begin();it != rsIn.end(); it++)
	{
		std::map<unsigned long, const sim_mob::RoadSegment*>::iterator itFromSeg(sim_mob::RoadSegment::allSegments.find(it->from_section));
		std::map<unsigned long, const sim_mob::RoadSegment*>::iterator itToSeg(sim_mob::RoadSegment::allSegments.find(it->to_section));
		if (itFromSeg != sim_mob::RoadSegment::allSegments.end() && itToSeg != sim_mob::RoadSegment::allSegments.end())
		{
			in.insert(std::make_pair(itFromSeg->second, itToSeg->second));
		}
		else
		{
			std::stringstream str("");
			str << "Section ids " << it->from_section << "," << it->to_section
					<< " has no candidate Road Segment among "
					<< sim_mob::RoadSegment::allSegments.size()
					<< " segments\n";
			throw std::runtime_error(str.str());
		}
	}

	const std::string& outTurningFunc = sim_mob::ConfigManager::GetInstance().FullConfig().getDatabaseProcMappings().
															procedureMappings["restricted_reg_out_turning"];

	soci::rowset<sim_mob::CBD_Pair> rsOut = sql.prepare << std::string("select * from ") + outTurningFunc;
	for (soci::rowset<sim_mob::CBD_Pair>::iterator it = rsOut.begin();	it != rsOut.end(); it++)
	{
		std::map<unsigned long, const sim_mob::RoadSegment*>::iterator itFromSeg(sim_mob::RoadSegment::allSegments.find(it->from_section));
		std::map<unsigned long, const sim_mob::RoadSegment*>::iterator itToSeg(sim_mob::RoadSegment::allSegments.find(it->to_section));

		if (itFromSeg != sim_mob::RoadSegment::allSegments.end() && itToSeg != sim_mob::RoadSegment::allSegments.end())
		{
			out.insert(std::make_pair(itFromSeg->second, itToSeg->second));
		}
		else
		{
			std::stringstream str("");
			str << "Section ids " << it->from_section << "," << it->to_section
					<< " has no candidate Road Segment among "
					<< sim_mob::RoadSegment::allSegments.size()
					<< " segments\n";
			throw std::runtime_error(str.str());
		}
	}
}

void DatabaseLoader::getCBD_Segments(const string & cnn, std::set<const sim_mob::RoadSegment*> & zoneSegments)
{
	soci::session sql(soci::postgresql, cnn);

	const std::string& restrictedRegSegFunc = sim_mob::ConfigManager::GetInstance().FullConfig().getDatabaseProcMappings().
															procedureMappings["restricted_reg_segments"];

	soci::rowset<int> rs = sql.prepare << std::string("select * from ") + restrictedRegSegFunc;
	for (soci::rowset<int>::iterator it = rs.begin();	it != rs.end(); it++)
	{
		std::map<unsigned long, const sim_mob::RoadSegment*>::iterator itSeg(sim_mob::RoadSegment::allSegments.find(*it));
		if(itSeg != sim_mob::RoadSegment::allSegments.end())
		{
			itSeg->second->CBD = true;
			zoneSegments.insert(itSeg->second);
		}
	}
}

void DatabaseLoader::getCBD_Nodes(const std::string& cnn, std::map<unsigned int, const sim_mob::Node*>& nodes)
{
	soci::session sql(soci::postgresql, cnn);

	const std::string& restrictedNodesFunc = sim_mob::ConfigManager::GetInstance().FullConfig().getDatabaseProcMappings().
															procedureMappings["restricted_reg_nodes"];

	soci::rowset<int> rs = sql.prepare << std::string("select * from ") + restrictedNodesFunc;
	for(soci::rowset<int>::iterator it = rs.begin(); it != rs.end(); it++)
	{
		std::map<unsigned int, const sim_mob::Node*>::iterator itNode = sim_mob::Node::allNodes.find((*it));
		if(itNode != sim_mob::Node::allNodes.end())
		{
			itNode->second->CBD = true;
			nodes[itNode->second->getID()] = itNode->second;
		}
	}
}

bool DatabaseLoader::InsertSinglePath2DB(soci::session& sql,std::set<sim_mob::SinglePath*,sim_mob::SinglePath>& spPool,const std::string pathSetTableName)
{
	BOOST_FOREACH(sim_mob::SinglePath* sp, spPool)
	{
		if(sp->isNeedSave2DB)
		{
			sql << "insert into " << pathSetTableName << "(id,pathset_id,partial_utility,path_size,signal_number,right_turn_number,scenario,length,highway_distance, min_distance,min_signal,min_right_turn,max_highway_usage, valid_path, shortest_path) "
					" values(:id,:pathset_id,:partial_utility,:path_size,:signal_number,:right_turn_number,:scenario,:length,:highway_distance, :min_distance,:min_signal,:min_right_turn,:max_highway_usage, :valid_path, :shortest_path)", soci::use(*sp);
			pathsetLogger << "insert into " << pathSetTableName << "\n";
		}
	}
}

void DatabaseLoader::loadPT_ChoiceSetFrmDB(soci::session& sql, std::string& pathSetId, sim_mob::PT_PathSet& pathSet)
{
	soci::rowset<sim_mob::PT_Path> rs = (sql.prepare << std::string("select * from get_pt_choiceset") + "(:pathset_id_in)", soci::use(pathSetId) );
	for (soci::rowset<sim_mob::PT_Path>::const_iterator it = rs.begin();	it != rs.end(); ++it) {
		pathSet.pathSet.insert(*it);
	}
}

void DatabaseLoader::LoadPT_PathsetFrmDB(soci::session& sql, const std::string& funcName, int originalNode, int destNode, sim_mob::PT_PathSet& pathSet)
{
	soci::rowset<sim_mob::PT_Path> rs = (sql.prepare << std::string("select * from ")+funcName + "(:o_node,:d_node)", soci::use(originalNode),soci::use(destNode) );
	for (soci::rowset<sim_mob::PT_Path>::const_iterator it = rs.begin();	it != rs.end(); ++it) {
		pathSet.pathSet.insert(*it);
	}
}
std::map<std::string, sim_mob::OneTimeFlag> ontimeFlog;
sim_mob::HasPath DatabaseLoader::loadSinglePathFromDB(soci::session& sql,
		std::string& pathset_id,
		std::set<sim_mob::SinglePath*, sim_mob::SinglePath>& spPool,
		const std::string functionName,
		const std::set<const sim_mob::RoadSegment*>& excludedRS)
{
	//prepare statement
	std::stringstream query;
	query << "select * from " << functionName << "(" << pathset_id << ")"; //pathset_id is a string of "<origin_node_id>,<destination_node_id>" format
	soci::rowset<sim_mob::SinglePath> rs = (sql.prepare	<< query.str());
	//	process result
	int cnt = 0;
	bool emptyCheck = true;
	for (soci::rowset<sim_mob::SinglePath>::const_iterator it = rs.begin();	it != rs.end(); ++it) {
		emptyCheck = false;
		bool proceed = true;
		std::vector<sim_mob::WayPoint> path = std::vector<sim_mob::WayPoint>();
		//use id to build shortestWayPointpath
		std::vector<std::string> segIds = std::vector<std::string>();
		boost::split(segIds, it->id, boost::is_any_of(","));
		// no path is correct
		for(int ii = 0 ; ii < segIds.size(); ++ii)
		{
			unsigned long id = 0;
			try
			{
				id = boost::lexical_cast<unsigned long> (segIds.at(ii));
				if(id > 0)
				{
					std::map<unsigned long, const sim_mob::RoadSegment*>::iterator it = sim_mob::RoadSegment::allSegments.find(id);
					const sim_mob::RoadSegment* seg = (it == sim_mob::RoadSegment::allSegments.end() ? nullptr : it->second);
					if(!seg)
					{
						std::string str = "SinglePath: seg not find " + id;
						throw std::runtime_error(str);
					}
					if(excludedRS.find(seg) != excludedRS.end())
					{
						proceed = false;
						break;
					}
					path.push_back(sim_mob::WayPoint(seg));//copy better than this twist
				}
				else
				{
					std::string str = "SinglePath: seg not find " + id;
					throw std::runtime_error(str);
				}
			}
			catch(std::exception &e)
			{
				if(ii  < (segIds.size()-1))//last comma
				{
					throw std::runtime_error(e.what());
				}
			}
		}
		if(!proceed)
		{
			pathsetLogger << "[PATHH: continue]\n";
			continue;
		}
		//create path object
		sim_mob::SinglePath *s = new sim_mob::SinglePath(*it);
		s->path = boost::move(path);
		if(s->path.empty())
		{
			throw std::runtime_error("Empty Path");
		}
		bool temp = spPool.insert(s).second;
		cnt++;
	}
	//due to limitations of soci, we could not use if(rs.begin() == rs.end()).. if(rs.empty) .. or if(rs.size() == 0)
	if(emptyCheck)
	{
		return sim_mob::PSM_NOTFOUND;
	}

	if (cnt == 0) {
		pathsetLogger << "DatabaseLoader::loadSinglePathFromDB: " << pathset_id << "no data in db\n" ;
		return sim_mob::PSM_NOGOODPATH;
	}
	return sim_mob::PSM_HASPATH;
}

void DatabaseLoader::loadLinkDefaultTravelTime(soci::session& sql,std::map<unsigned long, std::vector<sim_mob::LinkTravelTime> >& pool)
{
	const std::string &tableName = sim_mob::ConfigManager::GetInstance().PathSetConfig().DTT_Conf;
	std::string query = "select \"link_id\",to_char(\"start_time\",'HH24:MI:SS') AS start_time,"
			"to_char(\"end_time\",'HH24:MI:SS') AS end_time,\"travel_time\", travel_mode from \"" + tableName + "\"";

	soci::rowset<sim_mob::LinkTravelTime> rs = sql.prepare << query;

	for (soci::rowset<sim_mob::LinkTravelTime>::const_iterator itRS = rs.begin(); itRS!=rs.end(); ++itRS)  {
		pool[itRS->linkId].push_back(*itRS);
	}
}

bool DatabaseLoader::loadLinkRealTimeTravelTime(soci::session& sql, int intervalSec, sim_mob::AverageTravelTime& pool)
{
	int intervalMS = intervalSec * 1000;
	const std::string &tableName = sim_mob::ConfigManager::GetInstance().PathSetConfig().RTTT_Conf;
	std::string query = "select link_id,to_char(start_time,'HH24:MI:SS') AS start_time,"
			"to_char(end_time,'HH24:MI:SS') AS end_time,travel_time, travel_mode from \""
			+ tableName + "\" where interval_time = " + boost::lexical_cast<std::string>(intervalSec);

	//	local cache for optimization purposes
	std::map<unsigned long, const sim_mob::RoadSegment*> rsCache;
	std::map<unsigned long, const sim_mob::RoadSegment*>::iterator rsIt;

	//main loop
	try {
			unsigned int timeInterval;
			soci::rowset<sim_mob::LinkTravelTime> rs = (sql.prepare << query);
			for (soci::rowset<sim_mob::LinkTravelTime>::const_iterator it=rs.begin(); it!=rs.end(); ++it)  {
				timeInterval = sim_mob::TravelTimeManager::getTimeInterval(sim_mob::DailyTime(it->startTime).getValue(), intervalMS);
				//	optimization
				const sim_mob::RoadSegment* rs;
				if((rsIt = rsCache.find(it->linkId)) != rsCache.end())
				{
					rs = rsIt->second;
				}
				else
				{
					rs = rsCache[it->linkId] = sim_mob::RoadSegment::allSegments[it->linkId];
				}
				//the main job is just one line:
				pool[timeInterval][it->travelMode][rs] = it->travelTime;
			}
			return true;
	}
	catch (soci::soci_error const & err)
	{
		std::cout << "[ERROR LOADING REALTIME TRAVEL TIME]: " << err.what() << std::endl;
		return false;
	}
}

bool DatabaseLoader::CreateTable(soci::session& sql,std::string& tableName)
{
	try {
		sql  << ("CREATE TABLE "+tableName);
		sql.commit();
	}
	catch (soci::soci_error const & err)
	{
		std::cout<< "CreateTable: " << err.what() << std::endl;
		return false;
	}
	std::cout << "CreateTable: create table " << tableName << " ok"<< std::endl;
	return true;
}
bool DatabaseLoader::InsertData2TravelTimeTmpTable(std::string& tableName,
		sim_mob::LinkTravelTime& data)
{
	try {
		sql_<<"insert into "+ tableName +" (\"link_id\", \"start_time\",\"end_time\",\"travel_time\") "
							"values(:link_id, :start_time,:end_time,:travel_time)", soci::use(data);
		sql_.commit();
	}
	catch (soci::soci_error const & err)
	{
		std::cout<<"InsertData2TravelTimeTmpTable: "<<err.what()<<std::endl;
		return false;
	}
	return true;
}
bool DatabaseLoader::upsertTravelTime(soci::session& sql,const std::string& csvFileName, const std::string& tableName, double alpha)
{
	std::stringstream query("");
	query << "select * from " <<  sim_mob::ConfigManager::GetInstance().PathSetConfig().upsert  <<
			"('" << csvFileName << "','" << tableName << "'," << alpha << ");";
	std::cout << "executing query : " << query.str() << "\n";
	try
	{
		sql << query.str();
	}
	catch(std::exception &e){
		std::cout << "Error upserting the query( " << query.str() << " ) :\n" << e.what() << "\n";
	}
	return true;
}
bool DatabaseLoader::InsertCSV2Table(soci::session& sql,std::string& tableName,const std::string& csvFileName)
{
	try {
		sql << ("COPY " + tableName + " FROM '" + csvFileName + "' WITH DELIMITER AS ';'");
		sql.commit();
		}
		catch (soci::soci_error const & err)
		{
			std::cout << "InsertCSV2Table: " << err.what() <<std::endl;
			return false;
		}
		return true;
}

bool DatabaseLoader::TruncateTable(soci::session& sql,std::string& tableName)
{
	try {
		sql << "TRUNCATE TABLE "+ tableName;
		sql.commit();
	}
	catch (soci::soci_error const & err)
	{
		std::cout<<"TruncateTable: "<<err.what()<<std::endl;
		return false;
	}
	return true;
}
bool DatabaseLoader::ExcuString(soci::session& sql,std::string& str)
{
	try {
		sql << str;
		sql.commit();
	}
	catch (soci::soci_error const & err)
	{
		std::cout << "ExcuString: " << err.what() << std::endl;
		return false;
	}
	return true;
}
void DatabaseLoader::LoadERP_Surcharge(std::map<std::string,std::vector<sim_mob::ERP_Surcharge*> >& pool)
{
//	soci::rowset<sim_mob::ERP_Surcharge> rs = (sql_.prepare <<"select \"Gantry_No\",to_char(\"Start_Time\",'HH24:MI:SS') AS Start_Time,to_char(\"End _Time\",'HH24:MI:SS') AS End_Time,\"Rate\",\"Vehicle_Type_Id\",\"Vehicle_Type_Desc\",\"Day\" from \"ERP_Surcharge\" ");
	soci::rowset<sim_mob::ERP_Surcharge> rs = (sql_.prepare <<"select trim(both ' ' from \"Gantry_No\") AS Gantry_No,to_char(\"Start_Time\",'HH24:MI:SS') AS Start_Time,to_char(\"End _Time\",'HH24:MI:SS') AS End_Time,\"Rate\",\"Vehicle_Type_Id\",\"Vehicle_Type_Desc\",\"Day\" from \"ERP_Surcharge\" ");
	for (soci::rowset<sim_mob::ERP_Surcharge>::const_iterator it=rs.begin(); it!=rs.end(); ++it)  {
		sim_mob::ERP_Surcharge *s = new sim_mob::ERP_Surcharge(*it);
		std::map<std::string,std::vector<sim_mob::ERP_Surcharge*> >::iterator itt = pool.find(s->gantryNo);
		if(itt!=pool.end())
		{
			std::vector<sim_mob::ERP_Surcharge*> e = (*itt).second;
			e.push_back(s);
			pool[s->gantryNo] = e;
		}
		else
		{
			std::vector<sim_mob::ERP_Surcharge*> e;
			e.push_back(s);
			pool[s->gantryNo] = e;
		}
	}
}
void DatabaseLoader::LoadERP_Section(std::map<int,sim_mob::ERP_Section*>& ERP_SectionPool)
{
	soci::rowset<sim_mob::ERP_Section> rs = (sql_.prepare <<"select * from \"ERP_Section\" ");
	for (soci::rowset<sim_mob::ERP_Section>::const_iterator it=rs.begin(); it!=rs.end(); ++it)  {
		sim_mob::ERP_Section *s = new sim_mob::ERP_Section(*it);
		ERP_SectionPool.insert(std::make_pair(s->section_id,s));
	}
}
void DatabaseLoader::LoadERP_Gantry_Zone(std::map<std::string,sim_mob::ERP_Gantry_Zone*>& ERP_GantryZonePool)
{
	soci::rowset<sim_mob::ERP_Gantry_Zone> rs = (sql_.prepare <<"select * from \"ERP_Gantry_Zone\" ");
	for (soci::rowset<sim_mob::ERP_Gantry_Zone>::const_iterator it=rs.begin(); it!=rs.end(); ++it)  {
		sim_mob::ERP_Gantry_Zone *s = new sim_mob::ERP_Gantry_Zone(*it);
		ERP_GantryZonePool.insert(std::make_pair(s->gantryNo,s));
	}
}

void DatabaseLoader::LoadNodes(const std::string& storedProc)
{
	//Our SQL statement
	soci::rowset<Node> rs = (sql_.prepare <<"select * from " + storedProc);

	//Exectue as a rowset to avoid repeatedly building the query.
	nodes_.clear();
	for (soci::rowset<Node>::const_iterator it=rs.begin(); it!=rs.end(); ++it)  {
		if (nodes_.count(it->id)>0) {
			throw std::runtime_error("Duplicate AIMSUN node.");
		}

		//Convert meters to cm
		it->xPos *= 100;
		it->yPos *= 100;

		nodes_[it->id] = *it;
	}
}
void DatabaseLoader::loadSegmentTypeTable(const std::string& storedProc,std::map<string,int>& segTypeMap)
{
	try
	{
		soci::rowset<sim_mob::SegmentType> rs = (sql_.prepare <<"select * from " + storedProc);
		for (soci::rowset<sim_mob::SegmentType>::const_iterator it=rs.begin(); it!=rs.end(); ++it)  {
			segTypeMap.insert(std::make_pair(it->id,it->type));
		}
	}
	catch (soci::soci_error const & err)
	{
		std::cout<<"loadSegmentTypeTable: "<<err.what()<<std::endl;
	}
}
void DatabaseLoader::loadTurningSectionTable(const std::string& storedProc,sim_mob::RoadNetwork& rn) 
{
	try
	{
		if (storedProc.empty())
		{
			sim_mob::Warn() << "\nTurnings not loaded. Intersection behaviour will be affected.\n";
			return;
		}
		
		soci::rowset<sim_mob::TurningSection> turnings = (sql_.prepare <<"select * from " + storedProc);
		
		for (soci::rowset<sim_mob::TurningSection>::const_iterator it = turnings.begin(); it != turnings.end(); ++it)  
		{
			sim_mob::TurningSection *turning = new sim_mob::TurningSection(*it);
			rn.storeTurningSection(turning);
		}
	}
	catch (soci::soci_error const & err)
	{
		sim_mob::Print() << "loadTurningSectionTable: " << err.what() << std::endl;
	}
}
void DatabaseLoader::loadTurningPolyline(const std::string& storedProc, const std::string& pointsStoredProc, sim_mob::RoadNetwork& rn) 
{
	try
	{
		if(storedProc.empty())
		{
			sim_mob::Warn() << "\nTurning poly-lines not loaded. Intersection behaviour will be affected.\n";
			return;
		}
		
		soci::rowset<sim_mob::TurningPolyline> polylines = (sql_.prepare << "select * from " + storedProc);
		
		for (soci::rowset<sim_mob::TurningPolyline>::const_iterator it = polylines.begin(); it != polylines.end(); ++it)  
		{
			sim_mob::TurningPolyline *turningPolyline = new sim_mob::TurningPolyline(*it);
			loadPolypointByPolyline(pointsStoredProc, turningPolyline);
			rn.storeTurningPolyline(turningPolyline);
		}
	}
	catch (soci::soci_error const & err)
	{
		sim_mob::Print() << "loadTurningPolyline: " << err.what() << std::endl;
	}
}
void DatabaseLoader::loadPolypointByPolyline(const std::string& pointsStoredProc, sim_mob::TurningPolyline *turningPolyline) 
{
	try
	{
		std::stringstream s;
		s << "select * from " << pointsStoredProc << "(" << turningPolyline->getId() << ")";
		soci::rowset<sim_mob::Polypoint> polyLine = (sql_.prepare << s.str());
		
		for (soci::rowset<sim_mob::Polypoint>::const_iterator it = polyLine.begin(); it != polyLine.end(); ++it)  
		{
			sim_mob::Polypoint *point = new sim_mob::Polypoint(*it);
			point->x = point->x*100.0;
			point->y = point->y*100.0;
			turningPolyline->addPolypoint(point);
		}
	}
	catch (soci::soci_error const & err)
	{
		sim_mob::Print() << "loadTurningPolyline: " << err.what() << std::endl;
	}
}
void DatabaseLoader::storeTurningPoints(sim_mob::RoadNetwork& rn) 
{
	try 
	{
		std::string tableName = "TurningSection";

		std::map<std::string,sim_mob::TurningSection* >::iterator it;
		
		for(it = rn.turningSectionMap.begin(); it != rn.turningSectionMap.end(); ++it) 
		{
			sim_mob::TurningSection* ts = it->second;

			sql_<<"update \""+ tableName +"\" set from_xpos=:from_xpos ,from_ypos=:from_ypos,to_xpos=:to_xpos,to_ypos=:to_ypos"
					" where id=:id", soci::use(*ts);
			sql_.commit();
		}
	}
	catch (soci::soci_error const & err)
	{
		sim_mob::Print() << "storeTurningPoints: " << err.what() << std::endl;
	}
}
void DatabaseLoader::loadTurningConflictTable(const std::string& storedProc,sim_mob::RoadNetwork& rn) 
{
	try
	{
		if (storedProc.empty())
		{
			sim_mob::Warn() << "\nTurning conflicts not loaded. Intersection behaviour will be affected.\n";
			return;
		}
		
		soci::rowset<sim_mob::TurningConflict> conflicts = (sql_.prepare <<"select * from " + storedProc);
		for (soci::rowset<sim_mob::TurningConflict>::const_iterator it = conflicts.begin(); it != conflicts.end(); ++it)  
		{
			sim_mob::TurningConflict * conflict = new sim_mob::TurningConflict(*it);
			rn.storeTurningConflict(conflict);
		}
	}
	catch (soci::soci_error const & err)
	{
		sim_mob::Print() << "loadTurningConflictTable: " << err.what() << std::endl;
	}
}
void DatabaseLoader::loadNodeTypeTable(const std::string& storedProc,std::map<string,int>& nodeTypeMap)
{
	try{
		soci::rowset<sim_mob::NodeType> rs = (sql_.prepare <<"select * from " + storedProc);
		for (soci::rowset<sim_mob::NodeType>::const_iterator it=rs.begin(); it!=rs.end(); ++it)  {
			nodeTypeMap.insert(std::make_pair(it->id,it->type));
		}
	}
	catch (soci::soci_error const & err)
	{
		sim_mob::Print() << "loadNodeTypeTable: " << err.what() << std::endl;
	}
}

void DatabaseLoader::LoadSections(const std::string& storedProc)
{
	//Our SQL statement
	soci::rowset<Section> rs = (sql_.prepare <<"select * from " + storedProc);

	//Execute as a rowset to avoid repeatedly building the query.
	sections_.clear();
	for (soci::rowset<Section>::iterator it=rs.begin(); it!=rs.end(); ++it)  {
		//Check nodes
		if(nodes_.count(it->TMP_FromNodeID)==0 || nodes_.count(it->TMP_ToNodeID)==0) {
			sim_mob::Print() <<"From node: " <<it->TMP_FromNodeID  <<"  " <<nodes_.count(it->TMP_FromNodeID) <<"\n";
			sim_mob::Print() <<"To node: " <<it->TMP_ToNodeID  <<"  " <<nodes_.count(it->TMP_ToNodeID) <<"\n";
			throw std::runtime_error("Invalid From or To node.");
		}

		//Convert meters to cm
		it->length *= 100;

		//Note: Make sure not to resize the Node map after referencing its elements.
		it->fromNode = &nodes_[it->TMP_FromNodeID];
		it->toNode = &nodes_[it->TMP_ToNodeID];

		sections_[it->id] = *it;
	}
}

void DatabaseLoader::LoadPhase(const std::string& storedProc)
{
	//Optional
	if (storedProc.empty())
	{
		return;
	}

	soci::rowset<Phase> rs = (sql_.prepare <<"select * from " + storedProc);
	phases_.clear();
	int i=0;
	for(soci::rowset<Phase>::const_iterator it=rs.begin(); it!=rs.end(); ++it,i++)
	{
		map<int, Section>::iterator from = sections_.find(it->sectionFrom), to = sections_.find(it->sectionTo);
		//since the section index in sections_ and phases_ are read from two different tables, inconsistecy check is a must
		if((from ==sections_.end())||(to ==sections_.end()))
		{
			continue; //you are not in the sections_ container
		}

		it->ToSection = &sections_[it->sectionTo];
		it->FromSection = &sections_[it->sectionFrom];
		phases_.insert(pair<int,Phase>(it->nodeId,*it));
	}
}


void DatabaseLoader::LoadCrossings(const std::string& storedProc)
{
	if (storedProc.empty()) {
		return;
	}

	//Our SQL statement
	soci::rowset<Crossing> rs = (sql_.prepare <<"select * from " + storedProc);

	//Exectue as a rowset to avoid repeatedly building the query.
	crossings_.clear();
	for (soci::rowset<Crossing>::const_iterator it=rs.begin(); it!=rs.end(); ++it)  {
		//Check sections
		if(sections_.count(it->TMP_AtSectionID)==0) {
			sim_mob::Warn() <<"Crossing at Invalid Section\n";
			continue;
		}

		//Convert meters to cm
		it->xPos *= 100;
		it->yPos *= 100;

		//Note: Make sure not to resize the Section vector after referencing its elements.
		it->atSection = &sections_[it->TMP_AtSectionID];
		crossings_.push_back(*it);
	}
}

void DatabaseLoader::LoadLanes(const std::string& storedProc)
{
	if (storedProc.empty()) {
		return;
	}

	//Our SQL statement
	soci::rowset<Lane> rs = (sql_.prepare <<"select * from " + storedProc);

	//Exectue as a rowset to avoid repeatedly building the query.
	lanes_.clear();
	for (soci::rowset<Lane>::const_iterator it=rs.begin(); it!=rs.end(); ++it)  {
		//Check sections
		if(sections_.count(it->TMP_AtSectionID)==0) {
			sim_mob::Warn() <<"Lane at Invalid Section\n";
			continue;
		}

		//Convert meters to cm
		it->xPos *= 100;
		it->yPos *= 100;

		//Exclude "crossing" types
		if (it->laneType=="J" || it->laneType=="A4") {
			continue;
		}

		//Exclude lane markings which are not relevant to actual lane geometry
		if (it->laneType=="R" || it->laneType=="M" || it->laneType=="D" || it->laneType=="N"
				|| it->laneType=="Q" || it->laneType=="T" || it->laneType=="G" || it->laneType=="O"
						|| it->laneType=="A1" || it->laneType=="A3" || it->laneType=="L" || it->laneType=="H"
								|| it->laneType=="\\N"
		) {
			continue;
		}

		//Note: Make sure not to resize the Section vector after referencing its elements.
		it->atSection = &sections_[it->TMP_AtSectionID];
		lanes_.push_back(*it);
	}
}

/*
 * this function caters the section level not lane level
 * Turning contains 2 pairs (std::pair<int, int>) fromLane and toLane.
 * These respectively specify the range of lanes in the fromSection each of which is connected to every lane in range of lanes in toSection.
 * E.g: if fromLane is <0,1> and toLane<0,2>, then each of the lanes 0 and 1 in fromSection is connected to all of lanes 0,1,2 of toSection.
 * That is, the lane connections are 0-0, 0-1, 0-2, 1-0, 1-1, 1-2
 */
void DatabaseLoader::LoadTurnings(const std::string& storedProc)
{
	//Our SQL statement
	soci::rowset<Turning> rs = (sql_.prepare <<"select * from " + storedProc);

	//Exectue as a rowset to avoid repeatedly building the query.
	vector<int> skippedTurningIDs;
	turnings_.clear();
	for (soci::rowset<Turning>::const_iterator it=rs.begin(); it!=rs.end(); ++it)
	{
		if((sections_.count(it->TMP_FromSection)==0) || (sections_.count(it->TMP_ToSection)==0)) //Check nodes
		{
			skippedTurningIDs.push_back(it->id);
			continue;
		}

		//Note: Make sure not to resize the Section map after referencing its elements.
		it->fromSection = &sections_[it->TMP_FromSection];
		it->toSection = &sections_[it->TMP_ToSection];
		turnings_[it->id] = *it;
	}
}

void DatabaseLoader::LoadPolylines(const std::string& storedProc)
{
	//Our SQL statement
	soci::rowset<Polyline> rs = (sql_.prepare <<"select * from " + storedProc);

	//Exectue as a rowset to avoid repeatedly building the query.
	polylines_.clear();
	for (soci::rowset<Polyline>::const_iterator it=rs.begin(); it!=rs.end(); ++it)
	{
		//Check nodes
		if(sections_.count(it->TMP_SectionId)==0)
		{
			//throw std::runtime_error("Invalid polyline section reference.");
			sim_mob::Print() << "Invalid polyline section reference." << it->TMP_SectionId << std::endl;
			continue;
		}

		//Convert meters to cm
		it->xPos *= 100;
		it->yPos *= 100;

		//Note: Make sure not to resize the Section map after referencing its elements.
		it->section = &sections_[it->TMP_SectionId];
		polylines_.insert(std::make_pair(it->section->id, *it));
	}
}

void DatabaseLoader::LoadTripchains(const std::string& storedProc)
{
	//Avoid errors
	tripchains_.clear();
	if (storedProc.empty()) {
		return;
	}

	//Our SQL statement
	std::string sql_str = "select * from " + storedProc;

	//Load a different string if MPI is enabled.
#ifndef SIMMOB_DISABLE_MPI
	const sim_mob::ConfigParams& config = sim_mob::ConfigManager::GetInstance().FullConfig();
	if (config.using_MPI)
	{
		sim_mob::PartitionManager& partitionImpl = sim_mob::PartitionManager::instance();
		int partition_solution_id = partitionImpl.partition_config->partition_solution_id;

		//Note: partition_id starts from 1 while boost::mpi_id strats from 0
		int partition_id = partitionImpl.partition_config->partition_id + 1;

		std::string sqlPara = "";
		sqlPara += sim_mob::MathUtil::getStringFromNumber(partition_solution_id);
		sqlPara += ",";
		sqlPara += sim_mob::MathUtil::getStringFromNumber(partition_id);

		sql_str = "select * from get_trip_chains_in_partition(" + sqlPara + ")";

	}
#endif

	//Retrieve a rowset for this set of trip chains.
	soci::rowset<TripChainItem> rs = (sql_.prepare << sql_str);

	//Execute as a rowset to avoid repeatedly building the query.
	for (soci::rowset<TripChainItem>::const_iterator it=rs.begin(); it!=rs.end(); ++it)  {
		//The following are set regardless.
		it->startTime = sim_mob::DailyTime(it->tmp_startTime);

		//The following are only set for Trips or Activities respectively
		if(it->itemType == sim_mob::TripChainItem::IT_TRIP) {
			// check stops
			if(it->tripfromLocationType == sim_mob::TripChainItem::LT_PUBLIC_TRANSIT_STOP && it->triptoLocationType == sim_mob::TripChainItem::LT_PUBLIC_TRANSIT_STOP) {
				tripchains_.push_back(*it);continue;
			}
			//check nodes
			if (it->fromLocationType == sim_mob::TripChainItem::LT_NODE) {
				if (nodes_.count(it->tmp_fromLocationNodeID) == 0) {
					sim_mob::Print() << "Invalid trip chain fromNode reference."<< std::endl;
					//throw std::runtime_error("Invalid trip chain fromNode reference.");
					continue;
				} else {
					it->fromLocation = &nodes_[it->tmp_fromLocationNodeID];
				}
			}

			if (it->toLocationType == sim_mob::TripChainItem::LT_NODE) {
				if (nodes_.count(it->tmp_toLocationNodeID) == 0) {
					sim_mob::Print() << "Invalid trip chain toNode reference."<< std::endl;
					//throw std::runtime_error("Invalid trip chain toNode reference.");
					continue;
				} else {
					it->toLocation = &nodes_[it->tmp_toLocationNodeID];
				}
			}

		} else if(it->itemType == sim_mob::TripChainItem::IT_ACTIVITY) {
			//Set end time and location.
			it->endTime = sim_mob::DailyTime(it->tmp_endTime);
			it->location = &nodes_[it->tmp_locationID];
		} else {
			throw std::runtime_error("Unexpected trip chain type.");
		}

		//Finally, save it to our intermediate list of TripChainItems.
		tripchains_.push_back(*it);
	}
}

void
DatabaseLoader::LoadTrafficSignals(std::string const & storedProcedure)
{
    if (storedProcedure.empty()) {
        sim_mob::Warn() << "WARNING: An empty 'signal' stored-procedure was specified in the config file; "
                  << "will not lookup the database to create any signal found in there" << std::endl;
        return;
    }
    soci::rowset<Signal> rows = (sql_.prepare <<"select * from " + storedProcedure);
    for (soci::rowset<Signal>::const_iterator iter = rows.begin(); iter != rows.end(); ++iter) {
        // Convert from meters to centimeters.
        iter->xPos *= 100;
        iter->yPos *= 100;
        signals_.insert(std::make_pair(iter->id, *iter));
    }
}

void DatabaseLoader::LoadBusStop(const std::string& storedProc)
{
	//Bus stops are optional
	if (storedProc.empty()) { return; }

	soci::rowset<BusStop> rows = (sql_.prepare <<"select * from " + storedProc);
	for (soci::rowset<BusStop>::const_iterator iter = rows.begin(); iter != rows.end(); ++iter)
	{
		BusStop busstop = *iter;
		//Convert from meters to centimeters.
		busstop.xPos *= 100;
		busstop.yPos *= 100;
		busstop_.insert(std::make_pair(busstop.bus_stop_no, busstop));
	}
}

void DatabaseLoader::LoadBusStopSG(const std::string& storedProc)
{
	//Bus stops are optional
	if (storedProc.empty()) { return; }

	soci::rowset<BusStopSG> rows = (sql_.prepare <<"select * from " + storedProc);
	for (soci::rowset<BusStopSG>::const_iterator iter = rows.begin(); iter != rows.end(); ++iter)
	{
		BusStopSG busstop = *iter;
		// Convert from meters to centimeters.
		busstop.bus_stop_no.erase(remove_if(busstop.bus_stop_no.begin(), busstop.bus_stop_no.end(), ::isspace), busstop.bus_stop_no.end());
		busstop.stop_lat.erase(remove_if(busstop.stop_lat.begin(), busstop.stop_lat.end(), ::isspace), busstop.stop_lat.end());
		busstop.stop_lon.erase(remove_if(busstop.stop_lon.begin(), busstop.stop_lon.end(), ::isspace), busstop.stop_lon.end());

		busstop.xPos = boost::lexical_cast<double>(busstop.stop_lat) * 100;
		busstop.yPos = boost::lexical_cast<double>(busstop.stop_lon) * 100;
		busstop.xPos = boost::lexical_cast<double>(busstop.stop_lat);
		busstop.yPos = boost::lexical_cast<double>(busstop.stop_lon);
		bustopSG_.insert(std::make_pair(busstop.bus_stop_no, busstop));
	}
}

void DatabaseLoader::LoadPTBusDispatchFreq(const std::string& storedProc, std::vector<sim_mob::PT_BusDispatchFreq>& ptBusDispatchFreq)
{
	if (storedProc.empty())
	{
		sim_mob::Warn() << "WARNING: An empty 'PT_BusDispatchFreq' stored-procedure was specified in the config file; " << std::endl;
		return;
	}
	soci::rowset<sim_mob::PT_BusDispatchFreq> rows = (sql_.prepare <<"select * from " + storedProc);
	for (soci::rowset<sim_mob::PT_BusDispatchFreq>::const_iterator iter = rows.begin(); iter != rows.end(); ++iter)
	{
		//sim_mob::PT_bus_dispatch_freq* pt_bus_freqTemp = new sim_mob::PT_bus_dispatch_freq(*iter);
		sim_mob::PT_BusDispatchFreq pt_bus_freqTemp = *iter;
		pt_bus_freqTemp.routeId.erase(remove_if(pt_bus_freqTemp.routeId.begin(), pt_bus_freqTemp.routeId.end(), ::isspace),
				pt_bus_freqTemp.routeId.end());
		pt_bus_freqTemp.frequencyId.erase(remove_if(pt_bus_freqTemp.frequencyId.begin(), pt_bus_freqTemp.frequencyId.end(), ::isspace),
				pt_bus_freqTemp.frequencyId.end());
		ptBusDispatchFreq.push_back(pt_bus_freqTemp);
	}
}

void DatabaseLoader::LoadPTBusRoutes(const std::string& storedProc, std::vector<sim_mob::PT_BusRoutes>& pt_bus_routes, std::map<std::string, std::vector<const sim_mob::RoadSegment*> >& routeID_roadSegments)
{
	if (storedProc.empty())
	{
		sim_mob::Warn() << "WARNING: An empty 'pt_bus_routes' stored-procedure was specified in the config file; " << std::endl;
		return;
	}
	soci::rowset<sim_mob::PT_BusRoutes> rows = (sql_.prepare <<"select * from " + storedProc);
	for (soci::rowset<sim_mob::PT_BusRoutes>::const_iterator iter = rows.begin(); iter != rows.end(); ++iter)
	{
		sim_mob::PT_BusRoutes pt_bus_routesTemp = *iter;
		pt_bus_routes.push_back(pt_bus_routesTemp);
		sim_mob::RoadSegment *seg = sections_[atoi(pt_bus_routesTemp.linkId.c_str())].generatedSegment;
		if(seg) {
			routeID_roadSegments[iter->routeId].push_back(seg);
		}
	}
}

void DatabaseLoader::LoadPTBusStops(const std::string& storedProc, std::vector<sim_mob::PT_BusStops>& pt_bus_stops,
		std::map<std::string, std::vector<const sim_mob::BusStop*> >& routeID_busStops,
		std::map<std::string, std::vector<const sim_mob::RoadSegment*> >& routeID_roadSegments)
{
	sim_mob::ConfigParams& config = sim_mob::ConfigManager::GetInstanceRW().FullConfig();
	if (storedProc.empty())
	{
		sim_mob::Warn() << "WARNING: An empty 'pt_bus_stops' stored-procedure was specified in the config file; " << std::endl;
		return;
	}
	soci::rowset<sim_mob::PT_BusStops> rows = (sql_.prepare <<"select * from " + storedProc);
	for (soci::rowset<sim_mob::PT_BusStops>::const_iterator iter = rows.begin(); iter != rows.end(); ++iter)
	{
		sim_mob::PT_BusStops pt_bus_stopsTemp = *iter;
		pt_bus_stops.push_back(pt_bus_stopsTemp);

		sim_mob::BusStop* bs = sim_mob::BusStop::findBusStop(pt_bus_stopsTemp.stopNo);
		if(bs) {
			routeID_busStops[iter->routeId].push_back(bs);
		}
	}

	for(std::map<std::string, std::vector<const sim_mob::BusStop*> >::iterator routeIt=routeID_busStops.begin();
			routeIt!=routeID_busStops.end(); routeIt++)
	{
		std::map<std::string, std::vector<const sim_mob::RoadSegment*> >::iterator routeIDSegIt = routeID_roadSegments.find(routeIt->first);
		if(routeIDSegIt == routeID_roadSegments.end())
		{
			sim_mob::Warn() << routeIt->first << " has no route";
			continue;
		}
		std::vector<const sim_mob::BusStop*>& stopList = routeIt->second;
		std::vector<const sim_mob::RoadSegment*>& segList = routeIDSegIt->second;

		if(stopList.empty()) { throw std::runtime_error("empty stopList!"); }
		std::vector<const sim_mob::BusStop*> stopListCopy = stopList; //copy locally
		stopList.clear(); //empty stopList

		const sim_mob::BusStop* firstStop = stopListCopy.front();
		if(firstStop->terminusType == sim_mob::BusStop::SINK_TERMINUS)
		{
			const sim_mob::BusStop* firstStopTwin = firstStop->getTwinStop();
			if(!firstStopTwin) { throw std::runtime_error("Sink bus stop found without a twin!"); }
			stopList.push_back(firstStopTwin);
			if(!segList.empty())
			{
				std::vector<const sim_mob::RoadSegment*>::iterator itToDelete = segList.begin();
				while(itToDelete!=segList.end() && (*itToDelete) != firstStopTwin->getParentSegment())
				{
					itToDelete = segList.erase(itToDelete); // the bus must start from the segment of the twinStop
				}
				if(segList.empty())
				{
					throw std::runtime_error("Bus route violates terminus assumption. Entire route was deleted");
				}
			}
		}
		else
		{
			stopList.push_back(firstStop);
		}

		for(size_t stopIt = 1; stopIt < (stopListCopy.size()-1); stopIt++) //iterate through all stops but the first and last
		{
			const sim_mob::BusStop* stop = stopListCopy[stopIt];
			switch(stop->terminusType)
			{
				case sim_mob::BusStop::NOT_A_TERMINUS:
				{
					stopList.push_back(stop);
					break;
				}
				case sim_mob::BusStop::SOURCE_TERMINUS:
				{
					const sim_mob::BusStop* stopTwin = stop->getTwinStop();
					if(!stopTwin) { throw std::runtime_error("Source bus stop found without a twin!"); }
					stopList.push_back(stopTwin);
					stopList.push_back(stop);
					break;
				}
				case sim_mob::BusStop::SINK_TERMINUS:
				{
					const sim_mob::BusStop* stopTwin = stop->getTwinStop();
					if(!stopTwin) { throw std::runtime_error("Sink bus stop found without a twin!"); }
					stopList.push_back(stop);
					stopList.push_back(stopTwin);
					break;
				}
			}
		}

		const sim_mob::BusStop* lastStop = stopListCopy[stopListCopy.size()-1];
		if(lastStop->terminusType == sim_mob::BusStop::SOURCE_TERMINUS)
		{
			const sim_mob::BusStop* lastStopTwin = lastStop->getTwinStop();
			if(!lastStopTwin) { throw std::runtime_error("Source bus stop found without a twin!"); }
			stopList.pop_back();
			stopList.push_back(lastStopTwin);
			if(!segList.empty())
			{
				std::vector<const sim_mob::RoadSegment*>::iterator itToDelete = --segList.end();
				while((*itToDelete) != lastStopTwin->getParentSegment())
				{
					itToDelete = segList.erase(itToDelete); //the bus must end at the segment of twin stop
					itToDelete--; //itToDelete will be segList.end(); so decrement to get last valid iterator
				}
				if(segList.empty())
				{
					throw std::runtime_error("Bus route violates terminus assumption. Entire route was deleted");
				}
			}
		}
		else
		{
			stopList.push_back(lastStop);
		}
	}
}


void DatabaseLoader::LoadOD_Trips(const std::string& storedProc, std::vector<sim_mob::OD_Trip>& OD_Trips)
{
    if (storedProc.empty()) {
    	sim_mob::Warn() << "WARNING: An empty 'od_trips' stored-procedure was specified in the config file; "
               << "will not lookup the database to create any signal found in there" << std::endl;
        return;
    }
    soci::rowset<sim_mob::OD_Trip> rows = (sql_.prepare <<"select * from " + storedProc);
    for (soci::rowset<sim_mob::OD_Trip>::const_iterator iter = rows.begin(); iter != rows.end(); ++iter)
    {
    	OD_Trips.push_back(sim_mob::OD_Trip(*iter));
    }
}

std::string getStoredProcedure(map<string, string> const & storedProcs, string const & procedureName, bool mandatory=true)
{
	map<string, string>::const_iterator iter = storedProcs.find(procedureName);
	if (iter != storedProcs.end())
		return iter->second;
	if (!mandatory) {
		sim_mob::Print() <<"Skipping optional database property: " + procedureName <<std::endl;
		return "";
	}
	throw std::runtime_error("expected to find stored-procedure named '" + procedureName
			+ "' in the config file");
}

#ifndef SIMMOB_DISABLE_MPI
void DatabaseLoader::LoadBoundarySegments()
{
	sim_mob::PartitionManager& partitionImpl = sim_mob::PartitionManager::instance();
	int partition_solution_id = partitionImpl.partition_config->partition_solution_id;

	//Note: partition_id starts from 1 while boost::mpi_id strats from 0
	int partition_id = partitionImpl.partition_config->partition_id + 1;

	std::string sqlPara = "";
	sqlPara += sim_mob::MathUtil::getStringFromNumber(partition_solution_id);
	sqlPara += ",";
	sqlPara += sim_mob::MathUtil::getStringFromNumber(partition_id);

	std::string sql_str = "select * from get_boundary_segments_in_partition(" + sqlPara + ")";
	soci::rowset<soci::row> rs = (sql_.prepare << sql_str);

	for (soci::rowset<soci::row>::const_iterator it = rs.begin(); it != rs.end(); it++)
	{
		soci::row const& row = *it;

		sim_mob::BoundarySegment* boundary = new sim_mob::BoundarySegment();

		boundary->cutLineOffset = row.get<double> (0);
		boundary->connected_partition_id = row.get<int> (1);
		boundary->responsible_side = row.get<int> (2);
		boundary->start_node_x = row.get<double> (3);
		boundary->start_node_y = row.get<double> (4);
		boundary->end_node_x = row.get<double> (5);
		boundary->end_node_y = row.get<double> (6);

		boundary_segments.push_back(boundary);
	}
}

void DatabaseLoader::TransferBoundaryRoadSegment()
{
	sim_mob::PartitionManager& partitionImpl = sim_mob::PartitionManager::instance();
	vector<sim_mob::BoundarySegment*>::iterator it = boundary_segments.begin();
	for (; it != boundary_segments.end(); it++)
	{
		int start_x = static_cast<int> ((*it)->start_node_x * 100 + 0.5);
		int start_y = static_cast<int> ((*it)->start_node_y * 100 + 0.5);
		int end_x = static_cast<int> ((*it)->end_node_x * 100 + 0.5);
		int end_y = static_cast<int> ((*it)->end_node_y * 100 + 0.5);

		//		int start_x = static_cast<int> ((*it)->start_node_x * 100 );
		//		int start_y = static_cast<int> ((*it)->start_node_y * 100 );
		//		int end_x = static_cast<int> ((*it)->end_node_x * 100 );
		//		int end_y = static_cast<int> ((*it)->end_node_y * 100 );

		sim_mob::Point2D start_point(start_x, start_y);
		sim_mob::Point2D end_point(end_x, end_y);

		(*it)->boundarySegment = sim_mob::getRoadSegmentBasedOnNodes(&start_point, &end_point);
		partitionImpl.loadInBoundarySegment((*it)->boundarySegment->getId(), (*it));
	}

}
#endif


void DatabaseLoader::LoadScreenLineSegmentIDs(const map<string, string>& storedProcs, std::vector<unsigned long>& screenLines)
{
	screenLines.clear();

	string storedProc = getStoredProcedure(storedProcs, "screen_line");

	soci::rowset<unsigned long> rs = (sql_.prepare << "select * from " + storedProc);

	soci::rowset<unsigned long>::const_iterator iter = rs.begin();
	for(; iter != rs.end(); iter++)
	{
		screenLines.push_back(*iter);
	}
}

void DatabaseLoader::LoadObjectsForShortTerm(map<string, string> const & storedProcs)
{
	LoadCrossings(getStoredProcedure(storedProcs, "crossing", false));
	LoadLanes(getStoredProcedure(storedProcs, "lane", false));
	LoadTripchains(getStoredProcedure(storedProcs, "tripchain", false));
	LoadTrafficSignals(getStoredProcedure(storedProcs, "signal", false));
	LoadPhase(getStoredProcedure(storedProcs, "phase", false));

	//add by xuyan
	//load in boundary segments (not finished!)
#ifndef SIMMOB_DISABLE_MPI
	const sim_mob::ConfigParams& config = sim_mob::ConfigManager::GetInstance().FullConfig();
	if (config.using_MPI) {
		LoadBoundarySegments();
	}
#endif

}

void DatabaseLoader::LoadTurningSection(const map<string,string>& storedProcs, sim_mob::RoadNetwork& rn)
{
	// load turnings and turning conflicts
	loadTurningSectionTable(getStoredProcedure(storedProcs, "turning_section", false), rn);
	loadTurningConflictTable(getStoredProcedure(storedProcs, "turning_conflict", false), rn);
	loadTurningPolyline(getStoredProcedure(storedProcs, "turning_polyline", false),
						getStoredProcedure(storedProcs, "turning_polypoints", false), rn);

	// store turning start, to polyline points to db
	//storeTurningPoints(rn);
}

void DatabaseLoader::LoadBasicAimsunObjects(map<string, string> const & storedProcs)
{
	LoadNodes(getStoredProcedure(storedProcs, "node"));
	LoadSections(getStoredProcedure(storedProcs, "section"));
	LoadTurnings(getStoredProcedure(storedProcs, "turning"));
	LoadBusStop(getStoredProcedure(storedProcs, "busstop", false));
	LoadBusStopSG(getStoredProcedure(storedProcs, "busstopSG", false));
	LoadPolylines(getStoredProcedure(storedProcs, "polyline"));
}

void DatabaseLoader::loadObjectType(map<string, string> const & storedProcs,sim_mob::RoadNetwork& rn)
{
	loadSegmentTypeTable(getStoredProcedure(storedProcs, "segment_type"),rn.segmentTypeMap);
	loadNodeTypeTable(getStoredProcedure(storedProcs, "node_type"),rn.nodeTypeMap);
}

//Compute the distance from the source node of the polyline to a
// point on the line from the source to the destination nodes which
// is normal to the Poly-point.
void ComputePolypointDistance(Polyline& pt)
{
	//Our method is (fairly) simple.
	//First, compute the distance from the point to the polyline at a perpendicular angle.
	//
	// If the line passes through two points, (x1,y1) and (x2,y2),
	// and if we write Dx for (x2-x1) and Dy for (y2-y1),
	// the perpendicular distance from (x0,y0) to the line is given by:
	//
	// d = (Dy*x0 - Dx*y0 - x1y2 + x2y1) / (sqrt(Dx^2 + Dy^2))
	//
	// (x1,y1) is fromNode of section
	// (x2,y2) is toNode of section
	// (x0,y0) is the poly point pt
	double dx2x1 = pt.section->toNode->xPos - pt.section->fromNode->xPos;
	double dy2y1 = pt.section->toNode->yPos - pt.section->fromNode->yPos;
	double x1y2 = pt.section->fromNode->xPos * pt.section->toNode->yPos;
	double x2y1 = pt.section->toNode->xPos * pt.section->fromNode->yPos;
	double numerator = dy2y1*pt.xPos - dx2x1*pt.yPos - x1y2 + x2y1;
	double denominator = sqrt(dx2x1*dx2x1 + dy2y1*dy2y1);
	double perpenDist = numerator/denominator;
	if (perpenDist<0.0) {
		//We simplify all the quadratic math to just a sign change, since
		//   it's known that this polypoint has a positive distance to the line.
		perpenDist *= -1;
	}

	double dx1x0 = pt.section->fromNode->xPos - pt.xPos;
	double dy1y0 = pt.section->fromNode->yPos - pt.yPos;
	//Second, compute the distance from the source point to the polypoint
	double realDist = sqrt(dx1x0*dx1x0 + dy1y0*dy1y0);

	//Finally, apply the Pythagorean theorum
	pt.distanceFromSrc = sqrt(realDist*realDist - perpenDist*perpenDist);

	//NOTE: There simplest method would be to just take the x-component of the vector
	//      from pt.x/y to pt.section.fromNode.x/y, but you'd have to factor in
	//      the fact that this vector is rotated with respect to pt.section.from->pt.section.to.
	//      I can't remember enough vector math to handle this, but if anyone wants to
	//      replace it the vector version would certainly be faster. ~Seth
}



/**
 * Temporary functions.
 */
DynamicVector GetCrossingNearLine(Node& atNode, Node& toNode)
{
	//Get the outgoing set of crossing IDs
	map<Node*, vector<int> >::iterator outgoing = atNode.crossingLaneIdsByOutgoingNode.find(&toNode);
	if (outgoing!=atNode.crossingLaneIdsByOutgoingNode.end()) {
		//Search for the one closest to the toNode.
		DynamicVector resLine;
		double minDist = -1;
		for (vector<int>::iterator it2=outgoing->second.begin(); it2!=outgoing->second.end(); it2++) {
			//Get this item
			map<int, vector<Crossing*> >::iterator crossingIt = atNode.crossingsAtNode.find(*it2);
			if (crossingIt!=atNode.crossingsAtNode.end()) {
				//Now make a vector for it.
				DynamicVector currPoint(
						crossingIt->second.front()->xPos, crossingIt->second.front()->yPos,
						crossingIt->second.back()->xPos, crossingIt->second.back()->yPos
				);
				DynamicVector midPoint(currPoint);
				midPoint.scaleVectTo(midPoint.getMagnitude()/2).translateVect();
				double currDist = sim_mob::dist(midPoint.getX(), midPoint.getY(), toNode.xPos, toNode.yPos);
				if (minDist==-1 || currDist < minDist) {
					resLine = currPoint;
					minDist = currDist;
				}
			}
		}
		if (minDist > -1) {
			return resLine;
		}
	}
	throw std::runtime_error("Can't find crossing near line in temporary cleanup function.");
}
Section& GetSection(Node& start, Node& end)
{
	for (vector<Section*>::iterator it=start.sectionsAtNode.begin(); it!=start.sectionsAtNode.end(); it++) {
		if ((*it)->toNode->id==end.id) {
			return **it;
		}
	}
	sim_mob::Warn() <<"Error finding section from " <<start.id <<" to " <<end.id <<std::endl;
	throw std::runtime_error("Can't find section in temporary cleanup function.");
}
void ScaleLanesToCrossing(Node& start, Node& end, bool scaleEnd)
{
	//Retrieve the section
	Section& sect = GetSection(start, end);

	//Retrieve the crossing's "near" line.
	DynamicVector endLine = GetCrossingNearLine(scaleEnd?end:start, scaleEnd?start:end);

	//We can't do much until lanes are generated (we could try to guess what our lane generator would
	// do, but it's easier to set a debug flag).
	//std::cout <<"Saving end line: " <<endLine.getX() <<"," <<endLine.getY() <<" ==> " <<endLine.getEndX() <<"," <<endLine.getEndY() <<"\n";
	if (scaleEnd) {
		sect.HACK_LaneLinesEndLineCut = endLine;
	} else {
		sect.HACK_LaneLinesStartLineCut = endLine;
	}
}
void ResizeTo2(vector<Crossing*>& vec)
{
	if (vec.size()<=2) {
		if (vec.size()==2) {
			return;
		}
		throw std::runtime_error("Can't resize if vector is empty or has only one element.");
	}

	vec[1] = vec.back();
	vec.resize(2, nullptr);
}
vector<Crossing*>& GetCrossing(Node& atNode, Node& toNode, size_t crossingID)
		{
	//Get the outgoing set of crossing IDs
	map<Node*, vector<int> >::iterator outgoing = atNode.crossingLaneIdsByOutgoingNode.find(&toNode);
	if (outgoing!=atNode.crossingLaneIdsByOutgoingNode.end()) {
		//Narrow down to the one we want.
		for (vector<int>::iterator it2=outgoing->second.begin(); it2!=outgoing->second.end(); it2++) {
			if (*it2 != static_cast<int>(crossingID)) {
				continue;
			}
			map<int, vector<Crossing*> >::iterator crossingIt = atNode.crossingsAtNode.find(*it2);
			if (crossingIt!=atNode.crossingsAtNode.end()) {
				return crossingIt->second;
			}
			break;
		}
	}
	throw std::runtime_error("Can't find crossing in temporary cleanup function.");
		}
bool RebuildCrossing(Node& atNode, Node& toNode, size_t baseCrossingID, size_t resCrossingID, bool flipLeft, unsigned int crossingWidthCM, unsigned int paddingCM)
{
	//Retrieve the base Crossing and the Crossing we will store the result in.
	vector<Crossing*>& baseCrossing = GetCrossing(atNode, toNode, baseCrossingID);
	vector<Crossing*>& resCrossing = GetCrossing(atNode, toNode, resCrossingID);

	//Manual resize may be required
	ResizeTo2(baseCrossing);
	ResizeTo2(resCrossing);

	try {
		//Set point 1:
		{
			DynamicVector vec(baseCrossing.front()->xPos, baseCrossing.front()->yPos, baseCrossing.back()->xPos, baseCrossing.back()->yPos);
			vec.scaleVectTo(paddingCM).translateVect().flipNormal(!flipLeft);
			vec.scaleVectTo(crossingWidthCM).translateVect();
			resCrossing.front()->xPos = vec.getX();
			resCrossing.front()->yPos = vec.getY();
		}

		//Set point 2:
		{
			DynamicVector vec(baseCrossing.back()->xPos, baseCrossing.back()->yPos, baseCrossing.front()->xPos, baseCrossing.front()->yPos);
			vec.scaleVectTo(paddingCM).translateVect().flipNormal(flipLeft);
			vec.scaleVectTo(crossingWidthCM).translateVect();
			resCrossing.back()->xPos = vec.getX();
			resCrossing.back()->yPos = vec.getY();
		}
	} catch (std::exception& ex) {
		sim_mob::Warn() <<"Warning! Skipped crossing; error occurred (this should be fixed)." <<std::endl;
		baseCrossing.clear();
		resCrossing.clear();
		return false;
	}
	return true;

}
void ManuallyFixVictoriaStreetMiddleRoadIntersection(map<int, Node>& nodes, map<int, Section>& sections, vector<Crossing>& crossings, vector<Lane>& lanes, map<int, Turning>& turnings, multimap<int, Polyline>& polylines)
{
	//Step 1: Tidy up the crossings.
	/*RebuildCrossing(nodes[66508], nodes[93730], 683, 721, true, 450, 200);
	RebuildCrossing(nodes[66508], nodes[65120], 2419, 2111, false, 400, 200);
	RebuildCrossing(nodes[66508], nodes[75956], 3956, 3719, true, 450, 200);
	RebuildCrossing(nodes[66508], nodes[84882], 4579, 1251, true, 450, 200);

	//Step 2: Scale lane lines to match the crossings.
	ScaleLanesToCrossing(nodes[93730], nodes[66508], true);
	ScaleLanesToCrossing(nodes[66508], nodes[93730], false);
	ScaleLanesToCrossing(nodes[65120], nodes[66508], true);
	ScaleLanesToCrossing(nodes[66508], nodes[65120], false);
	ScaleLanesToCrossing(nodes[75956], nodes[66508], true);
	ScaleLanesToCrossing(nodes[66508], nodes[75956], false);
	ScaleLanesToCrossing(nodes[84882], nodes[66508], true);
	ScaleLanesToCrossing(nodes[66508], nodes[84882], false);*/
}




/**
 * Perform guided cleanup of the fully-loaded data. This step happens directly before the network is converted to
 * SimMobility format.
 *
 * \note
 * Currently, this process performs a single hard-coded check. Ideally, we would load data from another, smaller
 * database which contains a few "hints" to help nudge the various network components into the correct positions.
 * If you want a more heavy-handed approach, you should make a "PreProcessNetwork" function which does things like
 * deleting lanes, etc. (but be careful of invalidating references in that case).
 */
void DatabaseLoader::PostProcessNetwork()
{
	//This was a fix for a very old demo. Leaving it in here as an example of post-processing; you shouldn't use this. ~Seth.
	bool TEMP_FLAG_ON = false;
	if (TEMP_FLAG_ON) {
		ManuallyFixVictoriaStreetMiddleRoadIntersection(nodes_, sections_, crossings_, lanes_, turnings_, polylines_);
	}

}



sim_mob::Activity* MakeActivity(const TripChainItem& tcItem) {
	sim_mob::Activity* res = new sim_mob::Activity();
	res->setPersonID(tcItem.personID);
	res->itemType = tcItem.itemType;
	res->sequenceNumber = tcItem.sequenceNumber;
	res->description = tcItem.description;
	res->isPrimary = tcItem.isPrimary;
	res->isFlexible = tcItem.isFlexible;
	res->isMandatory = tcItem.isMandatory;
	res->location = tcItem.location->generatedNode;
	res->locationType = tcItem.locationType;
	res->startTime = tcItem.startTime;
	res->endTime = tcItem.endTime;
	res->travelMode = tcItem.mode;
	return res;
}


sim_mob::Trip* MakeTrip(const TripChainItem& tcItem) {
	sim_mob::ConfigParams& config = sim_mob::ConfigManager::GetInstanceRW().FullConfig();
	sim_mob::Trip* tripToSave = new sim_mob::Trip();
	tripToSave->tripID = tcItem.tripID;
	tripToSave->setPersonID(tcItem.personID);
	tripToSave->itemType = tcItem.itemType;
	tripToSave->sequenceNumber = tcItem.sequenceNumber;
	if(tcItem.fromLocationType == sim_mob::TripChainItem::LT_PUBLIC_TRANSIT_STOP) {
		std::string fromStop_no = boost::lexical_cast<std::string>(tcItem.tmp_fromLocationNodeID);
		sim_mob::BusStop* fromBusStop = config.getBusStopNo_BusStops()[fromStop_no];
		if(fromBusStop) {
			tripToSave->fromLocation = sim_mob::WayPoint(fromBusStop);
		} else {
			return nullptr;
		}
	} else {
		tripToSave->fromLocation = sim_mob::WayPoint( tcItem.fromLocation->generatedNode );
	}
	tripToSave->fromLocationType = tcItem.fromLocationType;
	tripToSave->startTime = tcItem.startTime;
	tripToSave->travelMode = tcItem.mode;
	return tripToSave;
}


bool FindBusLineWithLeastStops(Node* source, Node* destination, sim_mob::BusStop* & sourceStop, sim_mob::BusStop* & destStop)
{
	bool result = false;
	//sim_mob::AuraManager::instance2();
//	Point2D pnt1(source->getXPosAsInt()-3500, source->getYPosAsInt()-3500);
//	Point2D pnt2(source->getXPosAsInt()+3500, source->getYPosAsInt()+3500);
//	Point2D pnt1(source->xPos-35.00, source->yPos()-35.00);
//	Point2D pnt2(source->xPos+35.00, source->yPos+35.00);
	//std::vector<const sim_mob::Agent*> source_nearby_agents = sim_mob::AuraManager::instance2().agentsInRect(pnt1, pnt2, nullptr);

	std::vector<sim_mob::BusStop*> source_stops;
	std::vector<sim_mob::BusStop*> dest_stops;
	typedef std::pair<sim_mob::Busline*, sim_mob::BusStop*> LineToStop;
	typedef std::pair<LineToStop, LineToStop> LineToStopPair;
	std::vector< LineToStop > source_lines;
	std::vector< LineToStop > dest_lines;
	std::vector< LineToStopPair > selected_lines;
	std::vector<sim_mob::BusStop*>::iterator it;

	//find bus lines in source stop
	for(it = source_stops.begin(); it != source_stops.end(); it++)
	{
		std::vector<sim_mob::Busline*> buslines = (*it)->BusLines;
		std::vector<sim_mob::Busline*>::iterator itLines;
		for(itLines = buslines.begin(); itLines!=buslines.end(); itLines++)
		{
			source_lines.push_back(std::make_pair((*itLines), (*it)));
		}
	}

	//find bus lines in destination stop
	for(it = dest_stops.begin(); it != dest_stops.end(); it++)
	{
		std::vector<sim_mob::Busline*> buslines = (*it)->BusLines;
		std::vector<sim_mob::Busline*>::iterator itLines;
		for(itLines = buslines.begin(); itLines!=buslines.end(); itLines++)
		{
			dest_lines.push_back(std::make_pair((*itLines), (*it)));
		}
	}

	//find bus line which connect between source stop and destination stop with least number of stops
	std::vector<LineToStop>::iterator itSourceLineToStop, itDestLineToStop;
	for(itSourceLineToStop=source_lines.begin(); itSourceLineToStop!=source_lines.end(); itSourceLineToStop++)
	{
		sim_mob::Busline* source_line = itSourceLineToStop->first;
		for(itDestLineToStop=dest_lines.begin(); itDestLineToStop!=dest_lines.end(); itDestLineToStop++)
		{
			sim_mob::Busline* dest_line = itDestLineToStop->first;
			if( source_line->getBusLineID() == dest_line->getBusLineID() )
			{
				selected_lines.push_back(std::make_pair((*itSourceLineToStop), (*itDestLineToStop)));
			}
		}
	}

	//select bus line with least number of bus stops
	std::vector<LineToStopPair>::iterator itSelectedPair;
	sim_mob::BusStop* selSourceStop=0;
	sim_mob::BusStop* selDestStop=0;
	LineToStopPair selBusline;
	int minStops = 1000;
	for(itSelectedPair=selected_lines.begin(); itSelectedPair!=selected_lines.end(); itSelectedPair++)
	{
		LineToStop lineStopOne = itSelectedPair->first;
		LineToStop lineStopTwo = itSelectedPair->second;
		const std::vector<sim_mob::BusTrip>& BusTrips = (lineStopOne.first)->queryBusTrips();

		//bus stops has the info about the list of bus stops a particular bus line goes to
		std::vector<const sim_mob::BusStop*> busstops=BusTrips[0].getBusRouteInfo().getBusStops();
		std::vector<const sim_mob::BusStop*>::iterator it;
		int numStops = 0;
		for(it=busstops.begin(); it!=busstops.end(); it++)
		{
			if(numStops==0 && (*it)!=lineStopOne.second)
				continue;
			else if(numStops==0 && (*it)==lineStopOne.second)
				numStops++;
			else if(numStops>0 && (*it)!=lineStopTwo.second)
				numStops++;
			else if(numStops>0 && (*it)==lineStopTwo.second)
				break;
		}

		if(numStops < minStops)
		{
			minStops = numStops;
			selBusline = (*itSelectedPair);
			selSourceStop = lineStopOne.second;
			selDestStop = lineStopTwo.second;
			result = true;
		}
	}

	if(result)
	{
		sourceStop = selSourceStop;
		destStop = selDestStop ;
	}

	return result;
}

sim_mob::BusTrip* MakeBusTrip(const TripChainItem& tcItem, const std::map<std::string, std::vector<const sim_mob::BusStop*> >& route_BusStops,
	const std::map<std::string, std::vector<const sim_mob::RoadSegment*> >& route_RoadSegments) {

	//sim_mob::BusTrip* BusTripToSave = new sim_mob::BusTrip(tcItem.entityID,tcItem.sequenceNumber,tcItem.startTime,tcItem.endTime,tcItem.tripID);   //need Database Table connections(BusTripChainItem and BusTrip)
	sim_mob::BusTrip* BusTripToSave = nullptr;

	// MakeRoute
	std::string route_id = "";
	sim_mob::BusRouteInfo* busRouteInfo = new sim_mob::BusRouteInfo(route_id);

	const std::vector<const sim_mob::BusStop*>& busStop_vecTemp = route_BusStops.find(route_id)->second; // route_BusStops is a map loaded from database
	for(std::vector<const sim_mob::BusStop*>::const_iterator it = busStop_vecTemp.begin();it != busStop_vecTemp.end(); it++)
	{
		busRouteInfo->addBusStop(*it);
	}

	const std::vector<const sim_mob::RoadSegment*>& roadsegment_vecTemp = route_RoadSegments.find(route_id)->second; // route_RoadSegments is a map loaded from database
	for(std::vector<const sim_mob::RoadSegment*>::const_iterator it1 = roadsegment_vecTemp.begin();it1 != roadsegment_vecTemp.end(); it1++)
	{
		busRouteInfo->addRoadSegment(*it1);
	}


//	const std::vector<const sim_mob::BusStopInfo*>& busStopInfo_vecTemp = route_BusStopInfos.find(route_id)->second; // route_busStopScheduledTimes is a map loaded from database
//	for(std::vector<const sim_mob::BusStopInfo*>::const_iterator iter = busStopInfo_vecTemp.begin();iter != busStopInfo_vecTemp.end(); iter++)
//	{
//		busRouteInfo->addBusStopInfo(*iter);
//	}

	return BusTripToSave;

}

sim_mob::SubTrip MakeSubTrip(const TripChainItem& tcItem) {
	sim_mob::ConfigParams& config = sim_mob::ConfigManager::GetInstanceRW().FullConfig();
	sim_mob::SubTrip aSubTripInTrip;
	aSubTripInTrip.setPersonID(tcItem.personID);
	aSubTripInTrip.itemType = tcItem.itemType;
	aSubTripInTrip.tripID = tcItem.tmp_subTripID;
	if(tcItem.fromLocationType == sim_mob::TripChainItem::LT_PUBLIC_TRANSIT_STOP) {
		std::string fromStop_no = boost::lexical_cast<std::string>(tcItem.tmp_fromLocationNodeID);
		sim_mob::BusStop* fromBusStop = config.getBusStopNo_BusStops()[fromStop_no];
		if(fromBusStop) {
			aSubTripInTrip.fromLocation = sim_mob::WayPoint(fromBusStop);
		}
	} else {
		aSubTripInTrip.fromLocation = sim_mob::WayPoint( tcItem.fromLocation->generatedNode );
	}
	aSubTripInTrip.fromLocationType = tcItem.fromLocationType;
	if(tcItem.toLocationType == sim_mob::TripChainItem::LT_PUBLIC_TRANSIT_STOP) {
		std::string toStop_no = boost::lexical_cast<std::string>(tcItem.tmp_toLocationNodeID);
		sim_mob::BusStop* toBusStop = config.getBusStopNo_BusStops()[toStop_no];
		aSubTripInTrip.toLocation = sim_mob::WayPoint(toBusStop);
	} else {
		aSubTripInTrip.toLocation = sim_mob::WayPoint( tcItem.toLocation->generatedNode );
	}
	aSubTripInTrip.toLocationType = tcItem.toLocationType;
	aSubTripInTrip.mode = tcItem.mode;
	aSubTripInTrip.isPrimaryMode = tcItem.isPrimaryMode;
	aSubTripInTrip.ptLineId = tcItem.ptLineId;
	aSubTripInTrip.startTime = tcItem.startTime;
	aSubTripInTrip.mode = tcItem.mode;
	return aSubTripInTrip;
}

void AddSubTrip(sim_mob::Trip* parent, const sim_mob::SubTrip& subTrip) {
	// Update the trip destination so that toLocation eventually points to the destination of the trip.
	if(!parent) {
		return;
	}
	parent->toLocation = subTrip.toLocation;
	parent->toLocationType = subTrip.toLocationType;

	if(subTrip.fromLocation.busStop_ && subTrip.toLocation.busStop_) {
		//Add it to the list.
		parent->addSubTrip(subTrip);
	}
}

void DatabaseLoader::DecorateAndTranslateObjects()
{
	//Step 1: Tag all Nodes with the Sections that meet there.
	for (map<int,Section>::iterator it=sections_.begin(); it!=sections_.end(); it++)
	{
		it->second.fromNode->sectionsAtNode.push_back(&(it->second));
		it->second.toNode->sectionsAtNode.push_back(&(it->second));
	}

	//Step 2: Tag all Nodes that might be "UniNodes". These fit the following criteria:
	//        1) In ALL sections that meet at this node, there are only two distinct nodes.
	//        2) Each of these distinct nodes has exactly ONE Segment leading "from->to" and one leading "to->from".
	//           This should take bi-directional Segments into account.
	//        3) All Segments share the same Road Name
	//        4) Optionally, there can be a single link in ONE direction, representing a one-way road.
	vector<int> nodeMismatchIDs;
	for (map<int,Node>::iterator it=nodes_.begin(); it!=nodes_.end(); it++)
	{
		Node* n = &it->second;
		n->candidateForSegmentNode = true; //Conditional pass

		//Perform both checks at the same time.
		pair<Node*, Node*> others(nullptr, nullptr);
		pair<unsigned int, unsigned int> flags(0, 0);  //1="from->to", 2="to->from"
		string expectedName;
		for (vector<Section*>::iterator it=n->sectionsAtNode.begin(); it!=n->sectionsAtNode.end(); it++)
		{
			//Get "other" node
			Node* otherNode = ((*it)->fromNode != n) ? (*it)->fromNode : (*it)->toNode;

			//Manage property one.
			unsigned int* flagPtr;
			if (!others.first || others.first == otherNode)
			{
				others.first = otherNode;
				flagPtr = &flags.first;
			}
			else if (!others.second || others.second == otherNode)
			{
				others.second = otherNode;
				flagPtr = &flags.second;
			}
			else
			{
				n->candidateForSegmentNode = false; //Fail
				break;
			}

			//Manage property two.
			unsigned int toFlag = ((*it)->toNode == n) ? 1 : 2;
			if (((*flagPtr) & toFlag) == 0)
			{
				*flagPtr = (*flagPtr) | toFlag;
			}
			else
			{
				n->candidateForSegmentNode = false; //Fail
				break;
			}

			//Manage property three.
			if (expectedName.empty())
			{
				expectedName = (*it)->roadName;
			}
			else if (expectedName != (*it)->roadName)
			{
				n->candidateForSegmentNode = false; //Fail
				break;
			}

			//Manage property three.
			if (expectedName.empty()) {
				expectedName = (*it)->roadName;
			} else if (expectedName != (*it)->roadName) {
				n->candidateForSegmentNode = false; //Fail
				break;
			}
		}

		//One final check
		if (n->candidateForSegmentNode) {
			bool flagMatch =   (flags.first==3 && flags.second==3)  //Bidirectional
									|| (flags.first==1 && flags.second==2)  //One-way
									|| (flags.first==2 && flags.second==1); //One-way

			n->candidateForSegmentNode = others.first && others.second && flagMatch;
		}

		//Generate warnings if this value doesn't match the expected "is intersection" value.
		//This is usually a result of a network being cropped.
		if (n->candidateForSegmentNode == n->isIntersection) {
			nodeMismatchIDs.push_back(n->id);
		}
	}

	//Print all node mismatches at once
	sim_mob::PrintArray(nodeMismatchIDs, std::cout, "UniNode/Intersection mismatches: ", "[", "]", ", ", 4);

	//Step 3: Tag all Sections with Turnings that apply to that Section
	for (map<int,Turning>::iterator it=turnings_.begin(); it!=turnings_.end(); it++) {
		it->second.fromSection->connectedTurnings.push_back(&(it->second));
		it->second.toSection->connectedTurnings.push_back(&(it->second));
	}

	//Step 4: Add polyline entries to Sections. As you do this, compute their distance
	//        from the origin ("from" node)
	for (map<int,Polyline>::iterator it=polylines_.begin(); it!=polylines_.end(); it++) {
		it->second.section->polylineEntries.push_back(&(it->second));
		ComputePolypointDistance(it->second);
	}

	//Step 4.5: Request the LaneLoader to tag some Lane-related data.
	LaneLoader::DecorateLanes(sections_, lanes_);

	//Steps 5,6: Request the CrossingsLoader to tag some Crossing-related data.
	CrossingLoader::DecorateCrossings(nodes_, crossings_);
}


//Another temporary function
void CutSingleLanePolyline(vector<Point2D>& laneLine, const DynamicVector& cutLine, bool trimStart)
{
	//Compute the intersection of our lane line and the crossing.
	Point2D intPt = sim_mob::LineLineIntersect(cutLine, laneLine.front(), laneLine.back());
	if (intPt.getX() == std::numeric_limits<int>::max()) {
		throw std::runtime_error("Temporary lane function is somehow unable to compute line intersections.");
	}

	//Now update either the first or last point
	laneLine[trimStart?0:laneLine.size()-1] = intPt;
}

void DatabaseLoader::saveTripChains(std::map<std::string, std::vector<sim_mob::TripChainItem*> >& tcs)
{
	sim_mob::Trip* tripToSave = nullptr;
	for (vector<TripChainItem>::const_iterator it=tripchains_.begin(); it!=tripchains_.end(); it++) {
		if (it->itemType == sim_mob::TripChainItem::IT_ACTIVITY) {
			//TODO: Person related work
			sim_mob::Activity* activityToSave = MakeActivity(*it);
			if (activityToSave) {
				tcs[it->personID].push_back(activityToSave);
			}
		} else if(it->itemType == sim_mob::TripChainItem::IT_TRIP) {
			//Trips are slightly more complicated. Each trip is composed of several sub-trips;
			//  the first sub-trip is also used to initialize the trip. All sub-trips will have the
			//  same TripID; however, we might have several trips in a row (and therefore need to break
			//  in between). The previous code for this was somewhat fragile (didn't capture all this
			//  possible behavior; coudl potentially lead to null pointer exceptions, and had a faulty
			//  iterator check *outside* the loop), so here we will try to build things iteratively.
			if (!tripToSave) {
				//We at least need a trip to hold this
				tripToSave = MakeTrip(*it);
			}

			//Now, make and add a sub-trip
			AddSubTrip(tripToSave, MakeSubTrip(*it));


			//Are we "done" with this trip? The only way to continue is if there is a trip with the same
			//  trip ID immediately following this.
			if (tripToSave) {
				bool done = true;
				vector<TripChainItem>::const_iterator next = it+1;
				if (next!=tripchains_.end() && next->itemType==sim_mob::TripChainItem::IT_TRIP && next->tripID==tripToSave->tripID) {
					done = false;
				}

				//If done, save it.
				if (done) {
					tcs[it->personID].push_back(tripToSave);
					tripToSave = nullptr;
				}
			}
		}
	}
}


void DatabaseLoader::SaveSimMobilityNetwork(sim_mob::RoadNetwork& res, std::map<std::string, std::vector<sim_mob::TripChainItem*> >& tcs)
{
	//First, Nodes. These match cleanly to the Sim Mobility data structures
	sim_mob::Warn() <<"Warning: Units are not considered when converting AIMSUN data.\n";
	for (map<int,Node>::iterator it=nodes_.begin(); it!=nodes_.end(); it++) {
		sim_mob::aimsun::Loader::ProcessGeneralNode(res, it->second);
		it->second.generatedNode->originalDB_ID.setProps("aimsun-id", it->first);

		// set node type
		std::string idStr = boost::lexical_cast<string>(it->first);
		sim_mob::SimNodeType nt = (sim_mob::SimNodeType)res.getNodeType(idStr);
		it->second.generatedNode->type = nt;
	}
	//Next, Links and RoadSegments. See comments for our approach.
	for (map<int,Section>::iterator it=sections_.begin(); it!=sections_.end(); it++) {
		if (!it->second.hasBeenSaved) {
			sim_mob::aimsun::Loader::ProcessSection(res, it->second);
		}
	}
	//Scan the vector to see if any skipped Sections were not filled in later.
	for (map<int,Section>::iterator it=sections_.begin(); it!=sections_.end(); it++) {
		if (!it->second.hasBeenSaved) {	throw std::runtime_error("Section was skipped."); }
		if (it->second.generatedSegment->originalDB_ID.getLogItem().empty()) { //A bit hackish...
			it->second.generatedSegment->originalDB_ID.setProps("aimsun-id", it->first);
		}
	}
	//Next, SegmentNodes (UniNodes), which are only partially initialized in the general case.
	for (map<int,Node>::iterator it=nodes_.begin(); it!=nodes_.end(); it++) {
		if (it->second.candidateForSegmentNode) {
			sim_mob::aimsun::Loader::ProcessUniNode(res, it->second);
		}
	}
	//Next, Turnings. These generally match up.
	sim_mob::Warn() <<"Warning: Lanes-Left-of-Divider incorrect when converting AIMSUN data.\n";
	for (map<int,Turning>::iterator it=turnings_.begin(); it!=turnings_.end(); it++) {
		sim_mob::aimsun::Loader::ProcessTurning(res, it->second);
	}
	//Next, save the Polylines. This is best done at the Section level
	for (map<int,Section>::iterator it=sections_.begin(); it!=sections_.end(); it++) {
		sim_mob::aimsun::Loader::ProcessSectionPolylines(res, it->second);
	}

	//Prune Crossings and convert to the "near" and "far" syntax of Sim Mobility. Also give it a "position", defined
	//   as halfway between the midpoints of the near/far lines, and then assign it as an Obstacle to both the incoming and
	//   outgoing RoadSegment that it crosses.
	for (map<int,Node>::iterator it=nodes_.begin(); it!=nodes_.end(); it++) {
		for (map<Node*, std::vector<int> >::iterator i2=it->second.crossingLaneIdsByOutgoingNode.begin(); i2!=it->second.crossingLaneIdsByOutgoingNode.end(); i2++) {
			CrossingLoader::GenerateACrossing(res, it->second, *i2->first, i2->second);
		}
	}

	//Prune lanes and figure out where the median is.
	// TODO: This should eventually allow other lanes to be designated too.
	LaneLoader::GenerateLinkLanes(res, nodes_, sections_);

	sim_mob::aimsun::Loader::FixupLanesAndCrossings(res);

	// Create BusStopAgents based on the Bus Stops
	createBusStopAgents();
	//Save all trip chains
	saveTripChains(tcs);

	/*vahid:
	 * and Now we extend the signal functionality by adding extra information for signal's split plans, offset, cycle length, phases
	 * lots of these data are still default(cycle length, offset, choice set.
	 * They will be replaced by more realistic value(and input feeders) as the project proceeeds
	 */
	createSignals();
}

void
DatabaseLoader::createSignals()
{
    int j = 0, nof_signals = 0;
    for (map<int, Signal>::const_iterator iter = signals_.begin(); iter != signals_.end(); ++iter,j++)
    {
        Signal const & dbSignal = iter->second;
        map<int, Node>::const_iterator iter2 = nodes_.find(dbSignal.nodeId);

        //filter out signals which are not in the territory of our nodes_
        if (iter2 == nodes_.end())
        {
            std::ostringstream stream;
            stream << "cannot find node (id=" << dbSignal.nodeId
                   << ") in the database for signal id=" << iter->first;
            continue;
        }

        Node const & dbNode = iter2->second;
        sim_mob::Node const * node = dbNode.generatedNode;
    	//the traffic signal currently works with multinode only. so, although we have done conversions wherever necessary it
    	//would have been better to discard non multi nodes here only
    	if(!dynamic_cast<const sim_mob::MultiNode*>(node)) continue;
//        // There are 2 factors determining whether the following code fragment remains or should
//        // be deleted in the near future.  Firstly, in the current version, Signal is designed
//        // only for intersections with 4 links, the code in Signal.cpp and the visualizer expects
//        // to access 4 links and 4 crossings.  This needs to be fixed, Signal.cpp needs to be
//        // extended to model traffic signals at all kinds of intersections or at uni-nodes.
//        //
//        // However, even when Signal.cpp is fixed, the following code fragment may still remain
//        // here, although it may be modified.  The reason is that the entire road network may not
//        // be loaded.  There will be signal sites, especially at the edges of the loaded road
//        // networks, with missing links.  In some cases, it may not make any sense to create a
//        // Signal object there, even though a signal is present at that site in the database.
//        // One example is an intersection with 4 links, but only one link is loaded in.  That
//        // intersection would look like a dead-end to the Driver and Pedestrian objects.  Or
//        // an intersection with 4-way traffic, but only 3 links are loaded in.  This would "turn"
//        // the intersection into a T-junction.
//        std::set<sim_mob::Link const *> links;//links at the target node
//        if (sim_mob::MultiNode const * multi_node = dynamic_cast<sim_mob::MultiNode const *>(node))
//        {
//            std::set<sim_mob::RoadSegment*> const & roads = multi_node->getRoadSegments();
//            std::set<sim_mob::RoadSegment*>::const_iterator iter;
//            for (iter = roads.begin(); iter != roads.end(); ++iter)
//            {
//                sim_mob::RoadSegment const * road = *iter;
//                links.insert(road->getLink());//collecting links at the target node
//            }
//        }
//        if (links.size() != 4)//should change to what?
//        {
//            if (badNodes.count(node) == 0)
//            {
//                badNodes.insert(node);
//                std::cerr << "Hi, the node at " << node->location << " (database-id="
//                          << dbSignal.nodeId << ") does not have 4 links; "
//                          << "no signal will be created here." << std::endl;
//            }
//            continue;
//        }
        /*vahid:
         * ATTENTION: THIS PART OF THE FUNCTION ONWARDS IS DEPENDENT ON THE SCATS IMPLEMENTATION OF TRAFFIC SIGNAL
         * IF YOU ARE DEVELOPING A TRAFFIC SIGNAL BASED ON ANOTHER MODEL, YOU MUST CHANGED THIS PART ACCORDINGLY.
         * the following lines are the major tasks of this function(signalAt and addSignalSite functions)
         * the first line checks for availability of he signal in the street directory based on the node
         * (if not available, it will create a signal entry in the street directory)
         * the second line will add a signal site there are-on average- 16 sites for a signal
         * to clarify more the signal and signal site terms, think of a signal as a traffic controller box located at
         * an intersection. And think of signal site as the traffic light units installed at an intersection
         */
        //check validity of this signal cnadidate in terms of if availability of any phases
    	pair<multimap<int,sim_mob::aimsun::Phase>::iterator, multimap<int,sim_mob::aimsun::Phase>::iterator> ppp;
    	ppp = phases_.equal_range(node->getID()); //I repeate: Assumption is that node id and signal id are same
    	if(ppp.first == ppp.second)
    	{
    		continue;
    	}
    	bool isNew = false;
        const sim_mob::Signal_SCATS & signal = sim_mob::Signal_SCATS::signalAt(*node, sim_mob::ConfigManager::GetInstance().FullConfig().mutexStategy(), &isNew);
        //sorry I am calling the following function out of signal constructor. I am heavily dependent on the existing code
        //so sometimes a new functionality(initialize) needs to be taken care of separately
        //while it should be called with in other functions(constructor)-vahid
        if(isNew)
        {
        	createPlans(const_cast<sim_mob::Signal_SCATS &>(signal));
        	const_cast<sim_mob::Signal_SCATS &>(signal).initialize();
        	nof_signals++;
        }
        else
        	continue;
//		  not needed for the time being
//        const_cast<sim_mob::Signal &>(signal).addSignalSite(dbSignal.xPos, dbSignal.yPos, dbSignal.typeCode, dbSignal.bearing);
    }
    sim_mob::Print() << "signals created: " << nof_signals << std::endl;
}

/*SCATS IMPLEMENTATION ONLY.
 * prepares the plan member of signal class by assigning phases, choiceset and other parameters of the plan(splitplan)
 */
void
DatabaseLoader::createPlans(sim_mob::Signal_SCATS & signal)
{
	unsigned int sid ;
		sid = signal.getSignalId();//remember our assumption!  : node id and signal id(whtever their name is) are same
		sim_mob::SplitPlan & plan = signal.getPlan();
		plan.setParentSignal(&signal);
		createPhases(signal);

		//now that we have the number of phases, we can continue initializing our split plan.
		int nof_phases = signal.getNOF_Phases();
		if(nof_phases > 0)
		{
			if(nof_phases > 7)
				sim_mob::Print() << sid << " ignored due to lack of default choice set" << nof_phases ;
			else
			{
				plan.setDefaultSplitPlan(nof_phases);
			}
		}
		else
		{
			sim_mob::Print() << sid << " ignored due to no phases" << nof_phases <<  std::endl;
		}
}


void
DatabaseLoader::createPhases(sim_mob::Signal_SCATS & signal)
{

	pair<multimap<int,sim_mob::aimsun::Phase>::iterator, multimap<int,sim_mob::aimsun::Phase>::iterator> ppp;

	ppp = phases_.equal_range(signal.getSignalId());
	multimap<int,sim_mob::aimsun::Phase>::iterator ph_it = ppp.first;

	//some-initially weird looking- boost multi_index provisions to search for a phase by its name, instead of having loops to do that.
	sim_mob::Signal_SCATS::phases::iterator sim_ph_it;

	for(; ph_it != ppp.second; ph_it++)
	{
		sim_mob::Link * linkFrom = (*ph_it).second.FromSection->generatedSegment->getLink();
		sim_mob::Link * linkTo = (*ph_it).second.ToSection->generatedSegment->getLink();
		sim_mob::linkToLink ll(linkTo,(*ph_it).second.FromSection->generatedSegment,(*ph_it).second.ToSection->generatedSegment);
//		ll.RS_From = (*ph_it).second.FromSection->generatedSegment;
//		ll.RS_To = (*ph_it).second.ToSection->generatedSegment;
		std::string name = (*ph_it).second.name;
		for(sim_ph_it = signal.getPhases().begin(); sim_ph_it != signal.getPhases().end(); sim_ph_it++)
		{
			if(sim_ph_it->getName() == name)
				break;
		}
		if(sim_ph_it != signal.getPhases().end()) //means: if a phase with this name already exists in this plan...(usually u need a loop but with boost multi index, well, you don't :)
		{
			sim_ph_it->addLinkMapping(linkFrom,ll,dynamic_cast<sim_mob::MultiNode *>(nodes_[(*ph_it).second.nodeId].generatedNode));
		}
		else //new phase, new mapping
		{
			sim_mob::Phase phase(name,&(signal.getPlan()));//for general copy
			sim_mob::MultiNode * mNode = dynamic_cast<sim_mob::MultiNode *>(nodes_[(*ph_it).second.nodeId].generatedNode);
			if(!mNode)
			{
				std::cout << "We have a null MultiNode for " << nodes_[(*ph_it).second.nodeId].generatedNode->getID() << "here\n";
				continue;
			}
			phase.addLinkMapping(linkFrom,ll,mNode);
			phase.addDefaultCrossings(signal.getLinkAndCrossing(),mNode);
			signal.addPhase(phase);//congrates
		}
	}
}
} //End anon namespace

void DatabaseLoader::createBusStopAgents()
{
	//get stop capacity from genericProps
	int numBusesPerStop = 2;
	int numBusesPerTerminus = 10;
	try
	{
		std::string busPerStopStr = sim_mob::ConfigManager::GetInstance().FullConfig().system.genericProps.at("buses_per_stop");
		numBusesPerStop = std::atoi(busPerStopStr.c_str());
		if(numBusesPerStop < 1)
		{
			throw std::runtime_error("inadmissible value for buses per stop. Please check generic property 'buses_per_stop'");
		}
	}
	catch (const std::out_of_range& oorx)
	{
		sim_mob::Print() << "Generic property 'buses_per_stop' was not specified." << " Defaulting to " << numBusesPerStop << " buses." << std::endl;
	}

	std::map<std::string, sim_mob::BusStop*>& busStopMap = sim_mob::ConfigManager::GetInstanceRW().FullConfig().getBusStopNo_BusStops();

	//Save all bus stops
	for(map<std::string,BusStop>::iterator it = busstop_.begin(); it != busstop_.end(); it++)
	{
		std::map<int,Section>::iterator attachedSectionIt = sections_.find(it->second.TMP_AtSectionID);
		if(attachedSectionIt == sections_.end()) { continue; }

		//Create the bus stop
		sim_mob::BusStop* busstop = new sim_mob::BusStop();
		busstop->setParentSegment((*attachedSectionIt).second.generatedSegment);
		busstop->busstopno_ = it->second.bus_stop_no;
		busstop->busCapacityAsLength = BUS_LENGTH * numBusesPerStop;

		busstop->xPos = it->second.xPos;
		busstop->yPos = it->second.yPos;

		//Add the bus stop to its parent segment's obstacle list at an estimated offset.
		double distOrigin = sim_mob::BusStop::EstimateStopPoint(busstop->xPos, busstop->yPos, busstop->getParentSegment());
		if(!busstop->getParentSegment()->addObstacle(distOrigin, busstop)) {
			sim_mob::Warn() << "Can't add obstacle; something is already at that offset. " << busstop->busstopno_ << std::endl;
		}
		//set obstacle ID only after adding it to obstacle list.
		busstop->setRoadItemID(sim_mob::BusStop::generateRoadItemID(*(busstop->getParentSegment())));

		busStopMap[busstop->busstopno_] = busstop;
		sim_mob::BusStop::RegisterNewBusStop(busstop->busstopno_, busstop);

		//if current busstop is a terminus stop, we duplicate this stop and make one of them source and the other one as sink.
		//All buses ending at this terminus will end at the sink stop and all buses starting from the terminus will start from the source stop.
		//The source and sink stops are assumed to be in opposing but adjacent segments. If this assumption is violated, we might run into errors.

		if(it->second.TMP_RevSectionID != 0)
		{
			std::map<int,Section>::iterator revSectionIt = sections_.find(it->second.TMP_RevSectionID);
			if(revSectionIt != sections_.end())
			{
				map<int, Node>::iterator terminusNodeIt = nodes_.find(it->second.TMP_TerminalNodeID);
				if(terminusNodeIt==nodes_.end()) { throw std::runtime_error("node not found for terminus"); }
				const sim_mob::Node* terminusNode = terminusNodeIt->second.generatedNode;
				sim_mob::RoadSegment* reverseSectionForTerminus = (*revSectionIt).second.generatedSegment;
				sim_mob::BusStop* virtualStop = new sim_mob::BusStop();
				virtualStop->setVirtualStop();
				virtualStop->setParentSegment(reverseSectionForTerminus);
				virtualStop->busstopno_ = it->second.bus_stop_no + "_twin";
				virtualStop->busCapacityAsLength = BUS_LENGTH * numBusesPerTerminus;

				virtualStop->xPos = it->second.xPos;
				virtualStop->yPos = it->second.yPos;

				//Add the bus stop to its parent segment's obstacle list at an estimated offset.
				double distOrigin = sim_mob::BusStop::EstimateStopPoint(virtualStop->xPos, virtualStop->yPos, virtualStop->getParentSegment());
				if(!virtualStop->getParentSegment()->addObstacle(distOrigin, virtualStop)) {
					sim_mob::Warn() << "Can't add obstacle; something is already at that offset. " << virtualStop->busstopno_ << std::endl;
				}
				//set obstacle ID only after adding it to obstacle list.
				virtualStop->setRoadItemID(sim_mob::BusStop::generateRoadItemID(*(virtualStop->getParentSegment())));

				//more sanity checks
				if(busstop->getParentSegment() == virtualStop->getParentSegment())
				{
					throw std::runtime_error("invalid reverse section");
				}
				if(!((busstop->getParentSegment()->getStart() == terminusNode || busstop->getParentSegment()->getEnd() == terminusNode)
						&& (virtualStop->getParentSegment()->getStart() == terminusNode || virtualStop->getParentSegment()->getEnd() == terminusNode)))
				{
					throw std::runtime_error("invalid terminus node");
				}

				//now determine source and sink stops
				if(virtualStop->getParentSegment()->getStart() == terminusNode) // reverse section is downstream to attached section
				{
					virtualStop->terminusType = sim_mob::BusStop::SOURCE_TERMINUS;
					busstop->terminusType = sim_mob::BusStop::SINK_TERMINUS; //terminusNode must be the end node of the section for busstop
				}
				else
				{
					busstop->terminusType = sim_mob::BusStop::SOURCE_TERMINUS; //terminusNode must be the start node of the section for busstop
					virtualStop->terminusType = sim_mob::BusStop::SINK_TERMINUS;
				}

				busstop->busCapacityAsLength = BUS_LENGTH * numBusesPerTerminus; //update capacity of original stop as well
				busstop->setTwinStop(virtualStop);
				virtualStop->setTwinStop(busstop);
				busstop->getParentSegment()->setBusTerminusSegment();
				virtualStop->getParentSegment()->setBusTerminusSegment();

				busStopMap[virtualStop->busstopno_] = virtualStop;
				sim_mob::BusStop::RegisterNewBusStop(virtualStop->busstopno_, virtualStop);
			}
		}
	}

	for(map<std::string,BusStopSG>::iterator it = bustopSG_.begin(); it != bustopSG_.end(); it++) {
		std::map<int,Section>::iterator findPtr = sections_.find(it->second.aimsun_section);
		if(findPtr == sections_.end())
		{
			continue;
		}
		//Create the bus stop
		sim_mob::BusStop *busstop = new sim_mob::BusStop();
		sim_mob::RoadSegment* parentSeg = sections_[it->second.aimsun_section].generatedSegment;
		busstop->busstopno_ = it->second.bus_stop_no;
		busstop->busCapacityAsLength = BUS_LENGTH * numBusesPerStop;

		busstop->setParentSegment(parentSeg);

		busstop->xPos = it->second.xPos;
		busstop->yPos = it->second.yPos;

		//Add the bus stop to its parent segment's obstacle list at an estimated offset.
		double distOrigin = sim_mob::BusStop::EstimateStopPoint(busstop->xPos, busstop->yPos, sections_[it->second.aimsun_section].generatedSegment);
		if(!busstop->getParentSegment()->addObstacle(distOrigin, busstop)) {
			sim_mob::Warn() << "Can't add obstacle; something is already at that offset. " << busstop->busstopno_ << std::endl;
		}

		sim_mob::ConfigManager::GetInstanceRW().FullConfig().getBusStopNo_BusStops()[busstop->busstopno_] = busstop;

		//set obstacle ID only after adding it to obstacle list. For Now, it is how it works. sorry
		busstop->setRoadItemID(sim_mob::BusStop::generateRoadItemID(*(busstop->getParentSegment())));//sorry this shouldn't be soooo explicitly set/specified, but what to do, we don't have parent segment when we were creating the busstop. perhaps a constructor argument!?  :) vahid
		sim_mob::BusStop::RegisterNewBusStop(busstop->busstopno_, busstop);
	}
}

//Another temporary function
void sim_mob::aimsun::Loader::TMP_TrimAllLaneLines(sim_mob::RoadSegment* seg, const DynamicVector& cutLine, bool trimStart)
{
	//Nothing to do?
	if (cutLine.getMagnitude()==0.0) {
		return;
	}

	//Ensure that this segment has built all its lane lines.
	seg->syncLanePolylines();

	//Now go through and manually edit all of them. This includes lane lines and lane edge lines
	{
		vector< vector<Point2D> >::iterator it = seg->laneEdgePolylines_cached.begin();
		for (;it!=seg->laneEdgePolylines_cached.end(); it++) {
			CutSingleLanePolyline(*it, cutLine, trimStart);
		}
	}
	{
		vector<sim_mob::Lane*>::iterator it = seg->lanes.begin();
		for (;it!=seg->lanes.end(); it++) {

			CutSingleLanePolyline((*it)->polyline_, cutLine, trimStart);
		}
	}
}


void sim_mob::aimsun::Loader::FixupLanesAndCrossings(sim_mob::RoadNetwork& res)
{
	//Fix up lanes
	const std::vector<sim_mob::Link*>& vecLinks = res.getLinks();
	int numLinks = vecLinks.size();

	//TODO more comments needed
	//for(int n = 0; n < numLinks; ++n)
	for (std::vector<sim_mob::Link*>::const_iterator vIt = vecLinks.begin(); vIt!=vecLinks.end(); vIt++)
	{
		sim_mob::Link* link = *vIt;

		const std::vector<sim_mob::RoadSegment*>& fwdSegs = link->getSegments();
		std::set<sim_mob::RoadSegment*> roadSegs;
		roadSegs.insert(fwdSegs.begin(), fwdSegs.end());
		//roadSegs.insert(vecReverseSegs.begin(), vecReverseSegs.end());
		for(std::set<sim_mob::RoadSegment*>::const_iterator itRS = roadSegs.begin(); itRS!=roadSegs.end(); ++itRS)
		{
			for(std::map<sim_mob::centimeter_t, const sim_mob::RoadItem*>::const_iterator itObstacles = (*itRS)->obstacles.begin(); itObstacles != (*itRS)->obstacles.end(); ++itObstacles)
			{
				///TODO discuss constness of this variable on the RoadSegment and get rid of const cast
				sim_mob::RoadItem* ri = const_cast<sim_mob::RoadItem*>((*itObstacles).second);

				sim_mob::Crossing* cross = dynamic_cast<sim_mob::Crossing*>(ri);
				if(!cross)
					continue;

				//Due to some bugs upstream, certain crossings aren't found making the joins between crossing and lanes messy.
				//As an imperfect fix, make crossings rectangular.
				Point2D farLinemidPoint((cross->farLine.second.getX()-cross->farLine.first.getX())/2 + cross->farLine.first.getX(),(cross->farLine.second.getY()-cross->farLine.first.getY())/2 + cross->farLine.first.getY());
				Point2D nearLineProjection = ProjectOntoLine(farLinemidPoint, cross->nearLine.first, cross->nearLine.second);
				Point2D offset(farLinemidPoint.getX()-nearLineProjection.getX(),farLinemidPoint.getY()-nearLineProjection.getY());

				sim_mob::Point2D nearLinemidPoint((cross->nearLine.second.getX()-cross->nearLine.first.getX())/2 + cross->nearLine.first.getX(),
						(cross->nearLine.second.getY()-cross->nearLine.first.getY())/2 + cross->nearLine.first.getY());

				//Translate the crossing left or right to be centered on the link's median line.
				//This is imperfect but is an improvement.
				const sim_mob::Node* start = link->getStart();
				const sim_mob::Node* end = link->getEnd();
				if(	end && end->location.getX() !=0 && end->location.getY() !=0 &&
						start && start->location.getX() !=0 && start->location.getY() !=0)
				{
					sim_mob::Point2D medianProjection = LineLineIntersect(cross->nearLine.first, cross->nearLine.second, link->getStart()->location, link->getEnd()->location);
					Point2D shift(medianProjection.getX()-nearLinemidPoint.getX(), medianProjection.getY()-nearLinemidPoint.getY());
					///TODO this is needed temporarily due to a bug in which one intersection's crossings end up shifted across the map.
//					if(shift.getX() > 1000)
					if(shift.getX() > 10)
						continue;

					cross->nearLine.first = Point2D(cross->nearLine.first.getX()+shift.getX(), cross->nearLine.first.getY()+shift.getY());
					cross->nearLine.second = Point2D(cross->nearLine.second.getX()+shift.getX(), cross->nearLine.second.getY()+shift.getY());
					cross->farLine.first = Point2D(cross->farLine.first.getX()+shift.getX(), cross->farLine.first.getY()+shift.getY());
					cross->farLine.second = Point2D(cross->farLine.second.getX()+shift.getX(), cross->farLine.second.getY()+shift.getY());
				}

				std::vector<sim_mob::Point2D>& segmentPolyline = (*itRS)->polyline;
				{
					//Segment polyline
					double d1 = dist(segmentPolyline[0], nearLinemidPoint);
					double d2 = dist(segmentPolyline[segmentPolyline.size()-1], nearLinemidPoint);
					if (d2<d1)
					{
						segmentPolyline[segmentPolyline.size()-1] = ProjectOntoLine(segmentPolyline[segmentPolyline.size()-1], cross->farLine.first, cross->farLine.second);
					}
					else
					{
						segmentPolyline[0] = ProjectOntoLine(segmentPolyline[0], cross->farLine.first, cross->farLine.second);
					}
				}

				//Lane edge polylines
				//TODO don't access variable that should be private here
				std::vector< std::vector<sim_mob::Point2D> >& vecPolylines = (*itRS)->laneEdgePolylines_cached;
				for(size_t i = 0; i < vecPolylines.size(); ++i)
				{
					//TODO move this functionality into a helper function
					std::vector<sim_mob::Point2D>& vecThisPolyline = vecPolylines[i];
					double d1 = dist(vecThisPolyline[0], nearLinemidPoint);
					double d2 = dist(vecThisPolyline[vecThisPolyline.size()-1], nearLinemidPoint);
					if (d2<d1)
					{
						vecThisPolyline[vecThisPolyline.size()-1] = ProjectOntoLine(vecThisPolyline[vecThisPolyline.size()-1], cross->farLine.first, cross->farLine.second);
					}
					else
					{
						vecThisPolyline[0] = ProjectOntoLine(vecThisPolyline[0], cross->farLine.first, cross->farLine.second);
					}

				}
			}
		}
	}
}

void sim_mob::aimsun::Loader::ProcessGeneralNode(sim_mob::RoadNetwork& res, Node& src)
{
	sim_mob::Node* node = nullptr;
	if (!src.candidateForSegmentNode)
	{	//This is an Intersection
		sim_mob::MultiNode* multiNode = new sim_mob::Intersection(src.getXPosAsInt(), src.getYPosAsInt(), src.hasTrafficSignal);
		res.nodes.push_back(multiNode); //store it in the global nodes array
		node = multiNode;
	}
	else
	{
		sim_mob::UniNode* uniNode = new UniNode(src.getXPosAsInt(), src.getYPosAsInt());
		res.segmentnodes.insert(uniNode); //just save for later so the pointer is valid
		node = uniNode;
	}
	node->setID(src.id); //for future reference
	src.generatedNode = node;
	src.hasBeenSaved = true;
	//src.generatedNode->name = src.nodeName;
}

void sim_mob::aimsun::Loader::ProcessUniNode(sim_mob::RoadNetwork& res, Node& src)
{
	//Find 2 sections "from" and 2 sections "to".
	//(Bi-directional segments will complicate this eventually)
	//Most of the checks done here are already done earlier in the Loading process, but it doesn't hurt to check again.
	pair<Section*, Section*> fromSecs(nullptr, nullptr); //upstream sections
	pair<Section*, Section*> toSecs(nullptr, nullptr); //downstream sections
	for (vector<Section*>::iterator it=src.sectionsAtNode.begin(); it!=src.sectionsAtNode.end(); it++)
	{
		if ((*it)->TMP_ToNodeID==src.id)
		{
			if (!fromSecs.first) { fromSecs.first = *it; }
			else if (!fromSecs.second) { fromSecs.second = *it;}
			else { throw std::runtime_error("UniNode contains unexpected additional Sections leading TO.");	}
		}
		else if ((*it)->TMP_FromNodeID==src.id)
		{
			if (!toSecs.first) { toSecs.first = *it; }
			else if (!toSecs.second) { toSecs.second = *it; }
			else { throw std::runtime_error("UniNode contains unexpected additional Sections leading FROM."); }
		}
		else
		{
			throw std::runtime_error("UniNode contains a Section which actually does not lead to/from that Node.");
		}
	}

	//Ensure at least one path was found, and a non-partial second path.
	if (!(fromSecs.first && toSecs.first)) { throw std::runtime_error("UniNode contains no primary path."); }
	if ((fromSecs.second && !toSecs.second) || (!fromSecs.second && toSecs.second)) { throw std::runtime_error("UniNode contains partial secondary path."); }

	//This is a simple Road Segment joint
	UniNode* uniNode = dynamic_cast<UniNode*>(src.generatedNode);
	//newNode->location = new Point2D(src.getXPosAsInt(), src.getYPosAsInt());

	//Set locations (ensure unset locations are null)
	//Also ensure that we don't point backwards from the same segment.
	bool parallel = fromSecs.first->fromNode->id == toSecs.first->toNode->id;
	uniNode->firstPair.first = fromSecs.first->generatedSegment;
	uniNode->firstPair.second = parallel ? toSecs.second->generatedSegment : toSecs.first->generatedSegment;
	if (fromSecs.second && toSecs.second)
	{
		uniNode->secondPair.first = fromSecs.second->generatedSegment;
		uniNode->secondPair.second = parallel ? toSecs.first->generatedSegment : toSecs.second->generatedSegment;
	}
	else { uniNode->secondPair = pair<RoadSegment*, RoadSegment*>(nullptr, nullptr); }

	//TODO: Actual connector alignment (requires map checking)
	sim_mob::UniNode::buildConnectorsFromAlignedLanes(uniNode, std::make_pair(0, 0), std::make_pair(0, 0));
	//This UniNode can later be accessed by the RoadSegment itself.
}

void sim_mob::aimsun::Loader::ProcessSection(sim_mob::RoadNetwork& res, Section& src)
{
	//Skip Sections which start at a non-intersection. These will be filled in later.
	if (src.fromNode->candidateForSegmentNode) { return; }
	set<RoadSegment*> linkSegments;

	//Process this section, and continue processing Sections along the direction of
	// travel until one of these ends on an intersection.
	//NOTE: This approach is far from foolproof; for example, if a Link contains single-directional
	//      Road segments that fail to match at every UniNode. Need to find a better way to
	//      group RoadSegments into Links, but at least this works for our test network.
	Section* currSec = &src;  //Which section are we currently processing?
	sim_mob::Link* ln = new sim_mob::Link(1000001 + res.links.size());//max ten million links
	ln->roadName = currSec->roadName;
	ln->start = currSec->fromNode->generatedNode;

	//Make sure the link's start node is represented at the Node level.
	//TODO: Try to avoid dynamic casting if possible.
	for (;;)
	{
		//Update
		ln->end = currSec->toNode->generatedNode;

		//Check: not processing an existing segment
		if (currSec->hasBeenSaved) { throw std::runtime_error("Section processed twice."); }
		currSec->hasBeenSaved = true; //Mark saved		

		//Prepare a new segment IF required, and save it for later reference (or load from past ref.)
		if (!currSec->generatedSegment) 
		{ 
			currSec->generatedSegment = new sim_mob::RoadSegment(ln, currSec->id); 
		}

		//Save this segment if either end points are multinodes
		//TODO: This should be done at a global level, once the network has been loaded (similar to how XML does it).
		for (size_t tempID=0; tempID<2; tempID++)
		{
			sim_mob::Node* nd = tempID==0? currSec->fromNode->generatedNode : currSec->toNode->generatedNode;
			sim_mob::MultiNode* multNode = dynamic_cast<sim_mob::MultiNode*>(nd);
			if (multNode) { multNode->roadSegmentsAt.insert(currSec->generatedSegment); }
		}

		//Retrieve the generated segment
		sim_mob::RoadSegment* rs = currSec->generatedSegment;
		rs->originalDB_ID.setProps("aimsun-id", currSec->id);

		// set segment type
		std::string idStr = boost::lexical_cast<string>(currSec->id);
		sim_mob::SimSegmentType st = (sim_mob::SimSegmentType)res.getSegmentType(idStr);
		rs->type = st;

		//Start/end need to be added properly
		rs->start = currSec->fromNode->generatedNode;
		rs->end = currSec->toNode->generatedNode;

		//Process normal attributes; add empty lanes.
		rs->maxSpeed = currSec->speed;
		rs->length = currSec->length;
		rs->capacity = currSec->capacity;
		for (int laneID=0; laneID < currSec->numLanes; laneID++) {
			rs->lanes.push_back(new sim_mob::Lane(rs, laneID));
		}
		rs->width = 0;
		rs->highway = (currSec->serviceCategory == HIGHWAY_SERVICE_CATEGORY_STRING);

		//TODO: How do we determine if lanesLeftOfDivider should be 0 or lanes.size()
		//      In other words, how do we apply driving direction?
		//NOTE: This can be done easily later from the Link's point-of-view.
		rs->lanesLeftOfDivider = 0;
		linkSegments.insert(rs);

		//Break?
		if (!currSec->toNode->candidateForSegmentNode) {
			//Save it.
			ln->initializeLinkSegments(linkSegments);
			break;
		}

		//Increment.
		Section* nextSection = nullptr;
		for (vector<Section*>::iterator it2=currSec->toNode->sectionsAtNode.begin(); it2!=currSec->toNode->sectionsAtNode.end(); it2++) {
			//Our eariler check guarantees that there will be only ONE node which leads "from" the given segment "to" a node which is not the
			//  same node.
			if ((*it2)->fromNode==currSec->toNode && (*it2)->toNode!=currSec->fromNode) {
				if (nextSection) { throw std::runtime_error("UniNode has competing outgoing Sections."); }
				nextSection = *it2;
			}
		}
		if (!nextSection) {
			sim_mob::Warn() <<"PATH ERROR:\n"
							<<"  Starting at Node: " <<src.fromNode->id <<"\n"
							<<"  Currently at Node: " <<currSec->toNode->id <<"\n";
			throw std::runtime_error("No path reachable from RoadSegment.");
		}
		currSec = nextSection;
	}

	//Now add the link
	res.links.push_back(ln);
}


struct MyLaneConectorSorter {
  bool operator() ( const sim_mob::LaneConnector * c,  const sim_mob::LaneConnector * d) const
  {
	  if(!(c && d))
	  {
		  std::cout << "A lane connector is null\n";
		  return false;
	  }

	  const sim_mob::Lane* a = (c->getLaneFrom());
	  const unsigned int  aa = a->getRoadSegment()->getLink()->getLinkId();
	  const unsigned long  aaa = a->getRoadSegment()->getId();
	  const unsigned int  aaaa = a->getLaneID() ;

	  const sim_mob::Lane* b = (d->getLaneFrom());
	  const unsigned int  bb = b->getRoadSegment()->getLink()->getLinkId();
	  const unsigned long  bbb = b->getRoadSegment()->getId();
	  const unsigned int  bbbb = b->getLaneID() ;
	  ///////////////////////////////////////////////////////
	  const sim_mob::Lane* a1 = (c->getLaneTo());
	  const unsigned int  aa1 = a1->getRoadSegment()->getLink()->getLinkId();
	  const unsigned long  aaa1 = a1->getRoadSegment()->getId();
	  const unsigned int  aaaa1 = a1->getLaneID() ;

	  const sim_mob::Lane* b1 = (d->getLaneTo());
	  const unsigned int  bb1 = b1->getRoadSegment()->getLink()->getLinkId();
	  const unsigned long  bbb1 = b1->getRoadSegment()->getId();
	  const unsigned int  bbbb1 = b1->getLaneID() ;

	  if(!(a && b))
	  {
		  std::cout << "A lane from is null\n";
		  return false;
	  }
	  bool result = std::make_pair( aa, std::make_pair( aaa, std::make_pair(aaaa, std::make_pair( aa1, std::make_pair( aaa1, aaaa1 ) ))))
	        <
	        std::make_pair( bb, std::make_pair( bbb, std::make_pair(bbbb, std::make_pair( bb1, std::make_pair( bbb1, bbbb1 ) ))));

		  return result;
  }
} myLaneConnectorSorter;

void sim_mob::aimsun::Loader::ProcessTurning(sim_mob::RoadNetwork& res, Turning& src)
{
	//Check
	if (src.fromSection->toNode->id != src.toSection->fromNode->id) {
		//throw std::runtime_error("Turning doesn't match with Sections and Nodes.");
		std::cout << "Turning mismatch with Sections and Nodes|"
				<< " From " << src.fromSection->roadName << " (" << src.fromSection->fromNode->id << "," << src.fromSection->toNode->id << ")|"
				<< " To " << src.toSection->roadName << " (" << src.toSection->fromNode->id << "," << src.toSection->toNode->id << ")." << std::endl;
	}

	//Skip Turnings which meet at UniNodes; these will be handled elsewhere.
	sim_mob::Node* meetingNode = src.fromSection->toNode->generatedNode;
	if (dynamic_cast<UniNode*>(meetingNode)) { return; }

	//Essentially, just expand each turning into a set of LaneConnectors.
	//TODO: This becomes slightly more complex at RoadSegmentNodes, since these
	//      only feature one primary connector per Segment pair.
	for (int fromLaneID=src.fromLane.first; fromLaneID<=src.fromLane.second; fromLaneID++) {
		for (int toLaneID=src.toLane.first; toLaneID<=src.toLane.second; toLaneID++) {
			//Process
			sim_mob::LaneConnector* lc = new sim_mob::LaneConnector();
			int adjustedFromLaneId  = src.fromSection->generatedSegment->getAdjustedLaneId(fromLaneID);
			int adjustedToLaneId  = src.toSection->generatedSegment->getAdjustedLaneId(toLaneID);
			lc->laneFrom = src.fromSection->generatedSegment->lanes.at(adjustedFromLaneId);
			lc->laneTo = src.toSection->generatedSegment->lanes.at(adjustedToLaneId);
			//lc->laneFrom = src.fromSection->generatedSegment->lanes.at(fromLaneID);
			//lc->laneTo = src.toSection->generatedSegment->lanes.at(toLaneID);

			//just a check to avoid connecting pedestrian and non pedestrian lanes
			int i = 0;
			if(lc->laneFrom->is_pedestrian_lane()) { i++; }
			if(lc->laneTo->is_pedestrian_lane()) { i++; }

			if(i == 1) // it should be 0 or 2. i = 1 means only one of them is pedestrian lane
			{
				std::cout << "from Segment " << src.fromSection->generatedSegment->originalDB_ID.getLogItem();
				std::cout << ":lane " << 	fromLaneID ;
				std::cout << "  to Segment " << src.fromSection->generatedSegment->originalDB_ID.getLogItem();
				std::cout << ":lane " << 	toLaneID ;
				std::cout << "   has problem\n";
				delete lc;
				continue;
			}
			//check done ...

			//Expanded a bit...
			RoadSegment* key = src.fromSection->generatedSegment;
			map<const RoadSegment*, set<LaneConnector*> >& connectors = dynamic_cast<MultiNode*>(src.fromSection->toNode->generatedNode)->connectors;
			connectors[key].insert(lc);
		}
	}

}



void sim_mob::aimsun::Loader::ProcessSectionPolylines(sim_mob::RoadNetwork& res, Section& src)
{
	//The start point is first
	// NOTE: We agreed earlier to include the start/end points; I think this was because it makes lane polylines consistent. ~Seth
	{
		sim_mob::Point2D pt(src.fromNode->generatedNode->location);
		src.generatedSegment->polyline.push_back(pt);
	}

	//Polyline points are sorted by their "distance from source" and then added.
	std::sort(src.polylineEntries.begin(), src.polylineEntries.end(), polyline_sorter);
	for (std::vector<Polyline*>::iterator it=src.polylineEntries.begin(); it!=src.polylineEntries.end(); it++) {
		//TODO: This might not trace the median, and the start/end points are definitely not included.
		sim_mob::Point2D pt((*it)->xPos, (*it)->yPos);
		src.generatedSegment->polyline.push_back(pt);
	}

	//The ending point is last
	sim_mob::Point2D pt(src.toNode->generatedNode->location);
	src.generatedSegment->polyline.push_back(pt);
}


std::map<std::string, std::vector<sim_mob::TripChainItem*> > sim_mob::aimsun::Loader::LoadTripChainsFromNetwork(const string& connectionStr, const map<string, string>& storedProcs)
{
	std::cout << "Attempting to connect to database (TripChains)" << std::endl;
	DatabaseLoader loader(connectionStr);
	std::cout << ">Success." << std::endl;

	std::map<std::string, std::vector<sim_mob::TripChainItem*> > res;
	loader.LoadTripchains(getStoredProcedure(storedProcs, "tripchain"));
	loader.saveTripChains(res);
	return res;
}

void sim_mob::aimsun::Loader::getCBD_Border(
		std::set< std::pair<const sim_mob::RoadSegment*, const sim_mob::RoadSegment*> > &in,
		std::set< std::pair<const sim_mob::RoadSegment*, const sim_mob::RoadSegment*> > &out)
{
	std::string cnn(ConfigManager::GetInstance().FullConfig().getDatabaseConnectionString(false));
	DatabaseLoader::getCBD_Border(cnn, in, out);
}


void sim_mob::aimsun::Loader::getCBD_Segments(std::set<const sim_mob::RoadSegment*> & zoneSegments)
{
	std::string cnn(ConfigManager::GetInstance().FullConfig().getDatabaseConnectionString(false));
	DatabaseLoader::getCBD_Segments(cnn, zoneSegments);
}

void sim_mob::aimsun::Loader::getCBD_Nodes(std::map<unsigned int, const sim_mob::Node*>& nodes)
{
	std::string cnn(ConfigManager::GetInstance().FullConfig().getDatabaseConnectionString(false));
	DatabaseLoader::getCBD_Nodes(cnn, nodes);
}

void sim_mob::aimsun::Loader::LoadERPData(const std::string& connectionStr,
		std::map<std::string,std::vector<sim_mob::ERP_Surcharge*> > &ERP_SurchargePool,
		std::map<std::string,sim_mob::ERP_Gantry_Zone*>& ERP_GantryZonePool,
		std::map<int,sim_mob::ERP_Section*>& ERP_SectionPool)
{
	DatabaseLoader loader(connectionStr);
	loader.LoadERP_Surcharge(ERP_SurchargePool);
	loader.LoadERP_Section(ERP_SectionPool);
	loader.LoadERP_Gantry_Zone(ERP_GantryZonePool);
}
bool sim_mob::aimsun::Loader::createTable(soci::session& sql, std::string& tableName)
{
	return DatabaseLoader::CreateTable(sql, tableName);
}
bool sim_mob::aimsun::Loader::insertData2TravelTimeTmpTable(const std::string& connectionStr,
		std::string& tableName,
		sim_mob::LinkTravelTime& data)
{
	DatabaseLoader loader(connectionStr);
	bool res = loader.InsertData2TravelTimeTmpTable(tableName,data);
	return res;
}
bool sim_mob::aimsun::Loader::upsertTravelTime(soci::session& sql, const std::string& csvFileName, const std::string& tableName, double alpha)
{
	bool res = DatabaseLoader::upsertTravelTime(sql,csvFileName, tableName, alpha);
}
bool sim_mob::aimsun::Loader::insertCSV2Table(soci::session& sql, std::string& tableName, const std::string& csvFileName)
{
	bool res = DatabaseLoader::InsertCSV2Table(sql,tableName,csvFileName);
	return res;
}

bool sim_mob::aimsun::Loader::truncateTable(soci::session& sql,	std::string& tableName)
{
	bool res= DatabaseLoader::TruncateTable(sql, tableName);
	return res;
}
bool sim_mob::aimsun::Loader::excuString(soci::session& sql,std::string& str)
{
	bool res= DatabaseLoader::ExcuString(sql,str);
	return res;
}
void sim_mob::aimsun::Loader::LoadDefaultTravelTimeData(soci::session& sql,	std::map<unsigned long,std::vector<sim_mob::LinkTravelTime> >& linkDefaultTravelTimePool)
{
	DatabaseLoader::loadLinkDefaultTravelTime(sql, linkDefaultTravelTimePool);
}
bool sim_mob::aimsun::Loader::LoadRealTimeTravelTimeData(soci::session& sql, int interval,sim_mob::AverageTravelTime& linkRealtimeTravelTimePool)
{
	return DatabaseLoader::loadLinkRealTimeTravelTime(sql, interval, linkRealtimeTravelTimePool);
}


void sim_mob::aimsun::Loader::LoadPT_ChoiceSetFrmDB(soci::session& sql, std::string& pathSetId, sim_mob::PT_PathSet& pathSet)
{
	DatabaseLoader::loadPT_ChoiceSetFrmDB(sql, pathSetId, pathSet);
}

void sim_mob::aimsun::Loader::LoadPT_PathsetFrmDB(soci::session& sql, const std::string& funcName, int originalNode, int destNode, sim_mob::PT_PathSet& pathSet)
{
	DatabaseLoader::LoadPT_PathsetFrmDB(sql, funcName, originalNode, destNode, pathSet);
}

sim_mob::HasPath sim_mob::aimsun::Loader::loadSinglePathFromDB(soci::session& sql, std::string& pathset_id,
		std::set<sim_mob::SinglePath*, sim_mob::SinglePath>& spPool, const std::string functionName,
		const std::set<const sim_mob::RoadSegment *> & excludedRS)

{
	return DatabaseLoader::loadSinglePathFromDB(sql, pathset_id, spPool, functionName, excludedRS);
}

bool sim_mob::aimsun::Loader::storeSinglePath(soci::session& sql,
		std::set<sim_mob::SinglePath*, sim_mob::SinglePath>& pathPool,const std::string pathSetTableName)
{
	bool res = false;
	if(ConfigManager::GetInstance().PathSetConfig().privatePathSetMode == "generation")
	{
		sim_mob::BasicLogger & pathsetCSV = sim_mob::Logger::log(ConfigManager::GetInstance().PathSetConfig().bulkFile);
		BOOST_FOREACH(sim_mob::SinglePath* sp, pathPool)
		{
			if(sp->isNeedSave2DB)
			{
				pathsetCSV << ("\"" + sp->id + "\"") << ","
						<< ("\"" + sp->pathSetId + "\"") << ","
						<< sp->partialUtility << ","
						<< sp->pathSize << ","
						<< sp->signalNumber << ","
						<< sp->rightTurnNumber << ","
						<< ("\"" + sp->scenario  + "\"") << ","
						<< sp->length << ","
						<< sp->highWayDistance << ","
						<< sp->isMinDistance << ","
						<< sp->isMinSignal << ","
						<< sp->isMinRightTurn << ","
						<< sp->isMaxHighWayUsage << ","
						<< sp->valid_path << ","
						<< sp->isShortestPath << ","
						<< sp->travelTime << ","
						<< sp->isMinTravelTime << ","
						<< sp->pathSetId << "\n";
			}
		}
	}
	else
	{
		res = DatabaseLoader::InsertSinglePath2DB(sql,pathPool,pathSetTableName);
	}
	return res;
}

void sim_mob::aimsun::Loader::loadSegNodeType(const std::string& connectionStr, const std::map<std::string, std::string>& storedProcs, sim_mob::RoadNetwork& rn)
{
	DatabaseLoader loader(connectionStr);
	// load segment type data, node type data
	loader.loadObjectType(storedProcs,rn);
}

void sim_mob::aimsun::Loader::getScreenLineSegments(const std::string& connectionStr,
		const std::map<std::string, std::string>& storedProcs,
		std::vector<unsigned long>& screenLineList)
{
	DatabaseLoader loader(connectionStr);

	loader.LoadScreenLineSegmentIDs(storedProcs, screenLineList);
}

void sim_mob::aimsun::Loader::LoadNetwork(const string& connectionStr, const map<string, string>& storedProcs, sim_mob::RoadNetwork& rn, std::map<std::string, std::vector<sim_mob::TripChainItem*> >& tcs, ProfileBuilder* prof)
{
	std::cout << "Attempting to connect to database (generic)" << std::endl;

	//Connection string will look something like this:
	//"host=localhost port=5432 dbname=SimMobility_DB user=postgres password=XXXXX"
	DatabaseLoader loader(connectionStr);
	std::cout << ">Success." << std::endl;

	//Mutable config file reference.
	ConfigParams& config = ConfigManager::GetInstanceRW().FullConfig();

	//Step One: Load
	loader.LoadBasicAimsunObjects(storedProcs);

	if(!config.RunningMidSupply()) //TODO: add config for flag indicating short-term
	{
		// load data required for short-term
		loader.LoadObjectsForShortTerm(storedProcs);
	}


	//Step Two: Translate
	loader.DecorateAndTranslateObjects();

	//Step Three: Perform data-guided cleanup.
	loader.PostProcessNetwork();

	//Step Four: Save
	loader.SaveSimMobilityNetwork(rn, tcs);

	//Temporary Workaround: Links need to know about any "reverse" direction Links.
	//The StreetDirectory can provide this, but that hasn't been initialized yet.
	//So, we set a temporary flag in the Link class itself.
	map<std::pair<sim_mob::Node*, sim_mob::Node*>, sim_mob::Link*> startEndLinkMap;
	//Scan first.
	for (std::vector<sim_mob::Link*>::iterator linkIt=rn.getLinks().begin(); linkIt!=rn.getLinks().end(); linkIt++) {
		startEndLinkMap[std::make_pair((*linkIt)->getStart(), (*linkIt)->getEnd())] = *linkIt;
	}
	//Now tag
	for (std::vector<sim_mob::Link*>::iterator linkIt=rn.getLinks().begin(); linkIt!=rn.getLinks().end(); linkIt++) {
		bool hasOpp = startEndLinkMap.count(std::make_pair((*linkIt)->getEnd(), (*linkIt)->getStart()))>0;
		(*linkIt)->hasOpposingLink = hasOpp ? 1 : 0;
	}

	if(!config.RunningMidSupply()) //TODO: add config for flag indicating short-term
	{		//Temporary workaround; Cut lanes short/extend them as required.
		for (map<int,Section>::const_iterator it=loader.sections().begin(); it!=loader.sections().end(); it++) {
			TMP_TrimAllLaneLines(it->second.generatedSegment, it->second.HACK_LaneLinesStartLineCut, true);
			TMP_TrimAllLaneLines(it->second.generatedSegment, it->second.HACK_LaneLinesEndLineCut, false);
		}

		rn.makeSegPool();
		RoadNetwork::ForceGenerateAllLaneEdgePolylines(rn);
		loader.LoadTurningSection(storedProcs, rn);	

		// disabled by Max
		//for(vector<sim_mob::Link*>::iterator it = rn.links.begin(); it!= rn.links.end();it++)
		//{	(*it)->extendPolylinesBetweenRoadSegments(); }
	}

	//TODO: Possibly re-enable later.
	//if (prof) { prof->logGenericEnd("PostProc", "main-prof"); }

	for (map<int,Section>::const_iterator it=loader.sections().begin(); it!=loader.sections().end(); it++) {
		it->second.generatedSegment->computePolylineLength();
		it->second.generatedSegment->defaultTravelTime = it->second.generatedSegment->polylineLength/sim_mob::kmPerHourToCentimeterPerSecond(it->second.generatedSegment->maxSpeed);
	}

	//add by xuyan, load in boundary segments
	//Step Four: find boundary segment in road network using start-node(x,y) and end-node(x,y)
#ifndef SIMMOB_DISABLE_MPI
	if (ConfigManager::GetInstance().FullConfig().using_MPI)
	{
		loader.TransferBoundaryRoadSegment();
	}
#endif
	
	loader.LoadPTBusDispatchFreq(getStoredProcedure(storedProcs, "pt_bus_dispatch_freq", false), config.getPT_BusDispatchFreq());
	loader.LoadPTBusRoutes(getStoredProcedure(storedProcs, "pt_bus_routes", false), config.getPT_BusRoutes(), config.getRoadSegments_Map());
	loader.LoadPTBusStops(getStoredProcedure(storedProcs, "pt_bus_stops", false), config.getPT_BusStops(), config.getBusStops_Map(), config.getRoadSegments_Map());

	std::cout <<"AIMSUN Network successfully imported.\n";

}

void sim_mob::aimsun::Loader::CreateSegmentStats(const sim_mob::RoadSegment* rdSeg, std::list<sim_mob::SegmentStats*>& splitSegmentStats) {
	if(!rdSeg)
	{
		throw std::runtime_error("CreateSegmentStats(): NULL RoadSegment was passed");
	}
	std::stringstream debugMsgs;
	const std::map<sim_mob::centimeter_t, const sim_mob::RoadItem*>& obstacles = rdSeg->obstacles;
	double lengthCoveredInSeg = 0;
	double segStatLength;
	double rdSegmentLength = rdSeg->getPolylineLength();
	// NOTE: std::map implements strict weak ordering which defaults to less<key_type>
	// This is precisely the order in which we want to iterate the stops to create SegmentStats
	for(std::map<sim_mob::centimeter_t, const sim_mob::RoadItem*>::const_iterator obsIt = obstacles.begin(); obsIt != obstacles.end(); obsIt++)
	{
		const sim_mob::BusStop* busStop = dynamic_cast<const sim_mob::BusStop*>(obsIt->second);
		if(busStop)
		{
			double stopOffset = (double) (obsIt->first);
			if(stopOffset <= 0)
			{
				sim_mob::SegmentStats* segStats = new sim_mob::SegmentStats(rdSeg, rdSegmentLength);
				segStats->addBusStop(busStop);
				//add the current stop and the remaining stops (if any) to the end of the segment as well
				while(++obsIt != obstacles.end())
				{
					busStop = dynamic_cast<const sim_mob::BusStop*>(obsIt->second);
					if(busStop)
					{
						segStats->addBusStop(busStop);
					}
				}
				splitSegmentStats.push_back(segStats);
				lengthCoveredInSeg = rdSegmentLength;
				break;
			}
			if(stopOffset < lengthCoveredInSeg) {
				debugMsgs<<"bus stops are iterated in wrong order"
						<<"|seg: "<<rdSeg->getStartEnd()
						<<"|seg length: "<<rdSegmentLength
						<<"|curr busstop offset: "<<obsIt->first
						<<"|prev busstop offset: "<<lengthCoveredInSeg
						<<"|busstop: " << busStop->getBusstopno_()
						<<std::endl;
				throw std::runtime_error(debugMsgs.str());
			}
			if(stopOffset >= rdSegmentLength) {
				//this is probably due to error in data and needs manual fixing
				segStatLength = rdSegmentLength - lengthCoveredInSeg;
				lengthCoveredInSeg = rdSegmentLength;
				sim_mob::SegmentStats* segStats = new sim_mob::SegmentStats(rdSeg, segStatLength);
				segStats->addBusStop(busStop);
				//add the current stop and the remaining stops (if any) to the end of the segment as well
				while(++obsIt != obstacles.end()) {
					busStop = dynamic_cast<const sim_mob::BusStop*>(obsIt->second);
					if(busStop) {
						segStats->addBusStop(busStop);
					}
				}
				splitSegmentStats.push_back(segStats);
				break;
			}
			//the relation (lengthCoveredInSeg < stopOffset < rdSegmentLength) holds here
			segStatLength = stopOffset - lengthCoveredInSeg;
			lengthCoveredInSeg = stopOffset;
			sim_mob::SegmentStats* segStats = new sim_mob::SegmentStats(rdSeg, segStatLength);
			segStats->addBusStop(busStop);
			splitSegmentStats.push_back(segStats);
		}
	}

	// manually adjust the position of the stops to avoid short segments
	if(!splitSegmentStats.empty()) { // if there are stops in the segment
		//another segment stats has to be created for the remaining length.
		//this segment stats does not contain a bus stop
		//adjust the length of the last segment stats if the remaining length is short
		double remainingSegmentLength = rdSegmentLength - lengthCoveredInSeg;
		if(remainingSegmentLength < 0) {
			debugMsgs<<"Lengths of segment stats computed incorrectly\n";
			debugMsgs<<"segmentLength: "<<rdSegmentLength<<"|stat lengths: ";
			double totalStatsLength = 0;
			for(std::list<sim_mob::SegmentStats*>::iterator statsIt=splitSegmentStats.begin();
					statsIt!=splitSegmentStats.end(); statsIt++){
				debugMsgs<<(*statsIt)->length<<"|";
				totalStatsLength = totalStatsLength + (*statsIt)->length;
			}
			debugMsgs<<"totalStatsLength: "<<totalStatsLength<<std::endl;
			throw std::runtime_error(debugMsgs.str());
		}
		else if(remainingSegmentLength == 0) {
			// do nothing
		}
		else if(remainingSegmentLength < SHORT_SEGMENT_LENGTH_LIMIT) {
			// if the remaining length creates a short segment,
			// add this length to the last segment stats
			remainingSegmentLength = splitSegmentStats.back()->length + remainingSegmentLength;
			splitSegmentStats.back()->length = remainingSegmentLength;
		}
		else {
			// if the remaining length is long enough create a new SegmentStats
			sim_mob::SegmentStats* segStats = new sim_mob::SegmentStats(rdSeg, remainingSegmentLength);
			splitSegmentStats.push_back(segStats);
		}

		// if there is atleast 1 bus stop in the segment and the length of the
		// created segment stats is short, we will try to adjust the lengths to
		// avoid short segments
		bool noMoreShortSegs = false;
		while (!noMoreShortSegs && splitSegmentStats.size() > 1) {
			noMoreShortSegs = true; //hopefully
			sim_mob::SegmentStats* lastStats = splitSegmentStats.back();
			std::list<sim_mob::SegmentStats*>::iterator statsIt = splitSegmentStats.begin();
			while((*statsIt)!=lastStats) {
				SegmentStats* currStats = *statsIt;
				std::list<sim_mob::SegmentStats*>::iterator nxtStatsIt = statsIt; nxtStatsIt++; //get a copy and increment for next
				SegmentStats* nextStats = *nxtStatsIt;
				if(currStats->length < SHORT_SEGMENT_LENGTH_LIMIT) {
					noMoreShortSegs = false; //there is a short segment
					if(nextStats->length >= SHORT_SEGMENT_LENGTH_LIMIT) {
						double lengthDiff = SHORT_SEGMENT_LENGTH_LIMIT - currStats->length;
						currStats->length = SHORT_SEGMENT_LENGTH_LIMIT;
						nextStats->length = nextStats->length - lengthDiff;
					}
					else {
						// we will merge i-th SegmentStats with i+1-th SegmentStats
						// and add both bus stops to the merged SegmentStats
						nextStats->length = currStats->length + nextStats->length;
						for(std::vector<const sim_mob::BusStop*>::iterator stopIt=currStats->busStops.begin(); stopIt!=currStats->busStops.end(); stopIt++) {
							nextStats->addBusStop(*stopIt);
						}
						statsIt = splitSegmentStats.erase(statsIt);
						safe_delete_item(currStats);
						continue;
					}
				}
				statsIt++;
			}
		}
		if(splitSegmentStats.size() > 1) {
			// the last segment stat is handled separately
			std::list<sim_mob::SegmentStats*>::iterator statsIt = splitSegmentStats.end();
			statsIt--;
			SegmentStats* lastSegStats = *(statsIt);
			statsIt--;
			SegmentStats* lastButOneSegStats = *(statsIt);
			if(lastSegStats->length < SHORT_SEGMENT_LENGTH_LIMIT) {
				lastSegStats->length = lastButOneSegStats->length + lastSegStats->length;
				for(std::vector<const sim_mob::BusStop*>::iterator stopIt=lastButOneSegStats->busStops.begin();
						stopIt!=lastButOneSegStats->busStops.end(); stopIt++) {
					lastSegStats->addBusStop(*stopIt);
				}
				splitSegmentStats.erase(statsIt);
				safe_delete_item(lastButOneSegStats);
			}
		}
	}
	else {
		// if there are no stops in the segment, we create a single SegmentStats for this segment
		SegmentStats* segStats = new SegmentStats(rdSeg, rdSegmentLength);
		splitSegmentStats.push_back(segStats);
	}

	uint16_t statsNum = 1;
	std::set<sim_mob::SegmentStats*>& segmentStatsWithStops = ConfigManager::GetInstanceRW().FullConfig().getSegmentStatsWithBusStops();
	for(std::list<sim_mob::SegmentStats*>::iterator statsIt=splitSegmentStats.begin(); statsIt!=splitSegmentStats.end(); statsIt++) {
		SegmentStats* stats = *statsIt;
		//number the segment stats
		stats->statsNumberInSegment = statsNum;
		statsNum++;

		//add to segmentStatsWithStops if there is a bus stop in stats
		if (!(stats->getBusStops().empty())) {
			segmentStatsWithStops.insert(stats);
		}
	}
}

/*
 * iterates multinodes and creates confluxes for all of them
 */
// TODO: Remove debug messages
void sim_mob::aimsun::Loader::ProcessConfluxes(const sim_mob::RoadNetwork& rdnw) {
	std::stringstream debugMsgs(std::stringstream::out);
	std::set<sim_mob::Conflux*>& confluxes = ConfigManager::GetInstanceRW().FullConfig().getConfluxes();
	const sim_mob::MutexStrategy& mtxStrat = ConfigManager::GetInstance().FullConfig().mutexStategy();
	std::map<const sim_mob::MultiNode*, sim_mob::Conflux*>& multinode_confluxes
		= ConfigManager::GetInstanceRW().FullConfig().getConfluxNodes();

	//Make a temporary map of <multi node, set of road-segments directly connected to the multinode>
	//TODO: This should be done automatically *before* it's needed.
	std::map<const sim_mob::MultiNode*, std::set<const sim_mob::RoadSegment*> > roadSegmentsAt;
	for (std::vector<sim_mob::Link*>::const_iterator it=rdnw.links.begin(); it!=rdnw.links.end(); it++) {
		sim_mob::MultiNode* start = dynamic_cast<sim_mob::MultiNode*>((*it)->getStart());
		sim_mob::MultiNode* end = dynamic_cast<sim_mob::MultiNode*>((*it)->getEnd());
		if ((!start) || (!end)) { throw std::runtime_error("Link start/ends must be MultiNodes (in Conflux)."); }
		roadSegmentsAt[start].insert((*it)->getSegments().front());
		roadSegmentsAt[end].insert((*it)->getSegments().back());
	}

	for (vector<sim_mob::MultiNode*>::const_iterator i = rdnw.nodes.begin(); i != rdnw.nodes.end(); i++) {
		// we create a conflux for each multinode
		sim_mob::Conflux* conflux = new sim_mob::Conflux(*i, mtxStrat);
		try {
			std::set<const sim_mob::RoadSegment*>& segmentsAtNode = roadSegmentsAt.at(*i);
			if (!segmentsAtNode.empty()) {
				for (std::set<const sim_mob::RoadSegment*>::iterator segmtIt=segmentsAtNode.begin();
						segmtIt!=segmentsAtNode.end(); segmtIt++) {
					sim_mob::Link* lnk = (*segmtIt)->getLink();
					std::vector<sim_mob::SegmentStats*> upSegStatsList;
					if (lnk->getStart() == (*i))
					{
						//lnk is downstream to the multinode and doesn't belong to this conflux
						std::vector<sim_mob::RoadSegment*>& downSegs = lnk->getSegments();
						conflux->downstreamSegments.insert(downSegs.begin(), downSegs.end());
						if(lnk->getStart() != lnk->getEnd()) { continue; } // some links can start and end at the same section
					}
					//else
					//lnk *ends* at the multinode of this conflux.
					//lnk is upstream to the multinode and belongs to this conflux
					std::vector<sim_mob::RoadSegment*>& upSegs = lnk->getSegments();
					//set conflux pointer to the segments and create SegmentStats for the segment
					for(std::vector<sim_mob::RoadSegment*>::iterator segIt = upSegs.begin(); segIt != upSegs.end(); segIt++)
					{
						sim_mob::RoadSegment* rdSeg = *segIt;
						double rdSegmentLength = rdSeg->getPolylineLength();
						if(rdSeg->parentConflux == nullptr)
						{
							// assign only if not already assigned
							rdSeg->parentConflux = conflux;
							std::list<sim_mob::SegmentStats*> splitSegmentStats;
							CreateSegmentStats(rdSeg, splitSegmentStats);
							if(splitSegmentStats.empty()) {
								debugMsgs<<"no segment stats created for segment."
										 <<"|segment: "<<rdSeg->getStartEnd()
										 <<"|conflux: "<<conflux->multiNode
										 <<std::endl;
								throw std::runtime_error(debugMsgs.str());
							}
							std::vector<sim_mob::SegmentStats*>& rdSegSatsList = conflux->segmentAgents[rdSeg];
							rdSegSatsList.insert(rdSegSatsList.end(), splitSegmentStats.begin(), splitSegmentStats.end());
							upSegStatsList.insert(upSegStatsList.end(),splitSegmentStats.begin(), splitSegmentStats.end());
						}
						else if(rdSeg->parentConflux != conflux)
						{
							debugMsgs<<"\nProcessConfluxes\tparentConflux is being re-assigned for segment "
									<<rdSeg->getStartEnd()<<std::endl;
							throw std::runtime_error(debugMsgs.str());
						}
					}
					conflux->upstreamSegStatsMap.insert(std::make_pair(lnk, upSegStatsList));
					conflux->virtualQueuesMap.insert(std::make_pair(lnk, std::deque<sim_mob::Person*>()));
				} // end for
			} //end if
		}
		catch (const std::out_of_range& oor) {
			debugMsgs << "Loader::ProcessConfluxes() : No segments were found at multinode: "
					<< (*i)->getID() << "|location: " << (*i)->getLocation() << std::endl;
			sim_mob::Print() << debugMsgs.str();
			debugMsgs.str(std::string());
			continue;
		}
		conflux->resetOutputBounds();
		confluxes.insert(conflux);
		multinode_confluxes.insert(std::make_pair(*i, conflux));
	} // end for each multinode
	CreateLaneGroups();
}

void sim_mob::aimsun::Loader::CreateLaneGroups()
{
	std::set<sim_mob::Conflux*>& confluxes = ConfigManager::GetInstanceRW().FullConfig().getConfluxes();
	if(confluxes.empty()) { return; }

	typedef std::vector<sim_mob::SegmentStats*> SegmentStatsList;
	typedef std::map<const sim_mob::Lane*, sim_mob::LaneStats* > LaneStatsMap;
	typedef std::map<sim_mob::Link*, const SegmentStatsList> UpstreamSegmentStatsMap;

	for(std::set<sim_mob::Conflux*>::const_iterator cfxIt=confluxes.begin(); cfxIt!=confluxes.end(); cfxIt++)
	{
		UpstreamSegmentStatsMap& upSegsMap = (*cfxIt)->upstreamSegStatsMap;
		const sim_mob::MultiNode* cfxMultinode = (*cfxIt)->getMultiNode();
		for(UpstreamSegmentStatsMap::const_iterator upSegsMapIt=upSegsMap.begin(); upSegsMapIt!=upSegsMap.end(); upSegsMapIt++)
		{
			const SegmentStatsList& segStatsList = upSegsMapIt->second;
			if(segStatsList.empty()) { throw std::runtime_error("No segment stats for link"); }

			//assign downstreamLinks to the last segment stats
			SegmentStats* lastStats = segStatsList.back();
			const std::set<sim_mob::LaneConnector*>& lcs = cfxMultinode->getOutgoingLanes(lastStats->getRoadSegment());
			std::set<const sim_mob::Lane*> connLanes;
			for (std::set<sim_mob::LaneConnector*>::const_iterator lcIt = lcs.begin(); lcIt != lcs.end(); lcIt++)
			{
				const sim_mob::Lane* fromLane = (*lcIt)->getLaneFrom();
				connLanes.insert(fromLane);
				const sim_mob::Link* downStreamLink = (*lcIt)->getLaneTo()->getRoadSegment()->getLink();
				lastStats->laneStatsMap.at(fromLane)->addDownstreamLink(downStreamLink); //duplicates are eliminated by the std::set containing the downstream links
			}

			//construct inverse lookup for convenience
			for (LaneStatsMap::const_iterator lnStatsIt = lastStats->laneStatsMap.begin(); lnStatsIt != lastStats->laneStatsMap.end(); lnStatsIt++)
			{
				if(lnStatsIt->second->isLaneInfinity()) { continue; }
				const std::set<const sim_mob::Link*>& downstreamLnks = lnStatsIt->second->getDownstreamLinks();
				for(std::set<const sim_mob::Link*>::const_iterator dnStrmIt = downstreamLnks.begin(); dnStrmIt != downstreamLnks.end(); dnStrmIt++)
				{
					lastStats->laneGroup[*dnStrmIt].push_back(lnStatsIt->second);
				}
			}

			//extend the downstream links assignment to the segmentStats upstream to the last segmentStats
			SegmentStatsList::const_reverse_iterator upSegsRevIt = segStatsList.rbegin();
			upSegsRevIt++; //lanestats of last segmentstats is already assigned with downstream links... so skip the last segmentstats
			const sim_mob::SegmentStats* downstreamSegStats = lastStats;
			for(; upSegsRevIt!=segStatsList.rend(); upSegsRevIt++)
			{
				sim_mob::SegmentStats* currSegStats = (*upSegsRevIt);
				const sim_mob::RoadSegment* currSeg = currSegStats->getRoadSegment();
				const std::vector<sim_mob::Lane*>& currLanes = currSeg->getLanes();
				if(currSeg == downstreamSegStats->getRoadSegment())
				{	//currSegStats and downstreamSegStats have the same parent segment
					//lanes of the two segstats are same
					for (std::vector<sim_mob::Lane*>::const_iterator lnIt = currLanes.begin(); lnIt != currLanes.end(); lnIt++)
					{
						const sim_mob::Lane* ln = (*lnIt);
						if(ln->is_pedestrian_lane()) { continue; }
						const sim_mob::LaneStats* downStreamLnStats = downstreamSegStats->laneStatsMap.at(ln);
						sim_mob::LaneStats* currLnStats = currSegStats->laneStatsMap.at(ln);
						currLnStats->addDownstreamLinks(downStreamLnStats->getDownstreamLinks());
					}
				}
				else
				{
					const sim_mob::UniNode* uninode = dynamic_cast<const sim_mob::UniNode*>(currSeg->getEnd());
					if(!uninode) { throw std::runtime_error("Multinode found in the middle of a link"); }
					for (std::vector<sim_mob::Lane*>::const_iterator lnIt = currLanes.begin(); lnIt != currLanes.end(); lnIt++)
					{
						const sim_mob::Lane* ln = (*lnIt);
						if(ln->is_pedestrian_lane()) { continue; }
						sim_mob::LaneStats* currLnStats = currSegStats->laneStatsMap.at(ln);
						const UniNode::UniLaneConnector uniLnConnector = uninode->getForwardLanes(*ln);
						if(uniLnConnector.left)
						{
							const sim_mob::LaneStats* downStreamLnStats = downstreamSegStats->laneStatsMap.at(uniLnConnector.left);
							currLnStats->addDownstreamLinks(downStreamLnStats->getDownstreamLinks());
						}
						if(uniLnConnector.right)
						{
							const sim_mob::LaneStats* downStreamLnStats = downstreamSegStats->laneStatsMap.at(uniLnConnector.right);
							currLnStats->addDownstreamLinks(downStreamLnStats->getDownstreamLinks());
						}
						if(uniLnConnector.center)
						{
							const sim_mob::LaneStats* downStreamLnStats = downstreamSegStats->laneStatsMap.at(uniLnConnector.center);
							currLnStats->addDownstreamLinks(downStreamLnStats->getDownstreamLinks());
						}
					}
				}

				//construct inverse lookup for convenience
				for (LaneStatsMap::const_iterator lnStatsIt = currSegStats->laneStatsMap.begin(); lnStatsIt != currSegStats->laneStatsMap.end(); lnStatsIt++)
				{
					if(lnStatsIt->second->isLaneInfinity()) { continue; }
					const std::set<const sim_mob::Link*>& downstreamLnks = lnStatsIt->second->getDownstreamLinks();
					for(std::set<const sim_mob::Link*>::const_iterator dnStrmIt = downstreamLnks.begin(); dnStrmIt != downstreamLnks.end(); dnStrmIt++)
					{
						currSegStats->laneGroup[*dnStrmIt].push_back(lnStatsIt->second);
					}
				}

				downstreamSegStats = currSegStats;
			}

//			*********** the commented for loop below is to print the lanes which do not have lane groups ***
//			for(SegmentStatsList::const_reverse_iterator statsRevIt=segStatsList.rbegin(); statsRevIt!=segStatsList.rend(); statsRevIt++)
//			{
//				const LaneStatsMap lnStatsMap = (*statsRevIt)->laneStatsMap;
//				unsigned int segId = (*statsRevIt)->getRoadSegment()->getSegmentAimsunId();
//				uint16_t statsNum = (*statsRevIt)->statsNumberInSegment;
//				const std::vector<sim_mob::Lane*>& lanes = (*statsRevIt)->getRoadSegment()->getLanes();
//				unsigned int numLanes = 0;
//				for(std::vector<sim_mob::Lane*>::const_iterator lnIt = lanes.begin(); lnIt!=lanes.end(); lnIt++)
//				{
//					if(!(*lnIt)->is_pedestrian_lane()) { numLanes++; }
//				}
//				for (LaneStatsMap::const_iterator lnStatsIt = lnStatsMap.begin(); lnStatsIt != lnStatsMap.end(); lnStatsIt++)
//				{
//					if(lnStatsIt->second->isLaneInfinity() || lnStatsIt->first->is_pedestrian_lane()) { continue; }
//					if(lnStatsIt->second->getDownstreamLinks().empty())
//					{
//						Print() << "~~~ " << segId << "," << statsNum << "," << lnStatsIt->first->getLaneID() << "," << numLanes << std::endl;
//					}
//				}
//			}
		}
	}
}

sim_mob::BusStopFinder::BusStopFinder(const Node* src, const Node* dest)
{
	originBusStop = findNearbyBusStop(src);
    destBusStop = findNearbyBusStop(dest);
}

sim_mob::BusStop* sim_mob::BusStopFinder::findNearbyBusStop(const Node* node)
{
	 const MultiNode* currEndNode = dynamic_cast<const MultiNode*> (node);
	 double dist=0;
	 BusStop*bs1=0;
	 if(currEndNode)
	 {
		 const std::set<sim_mob::RoadSegment*>& segments_ = currEndNode->getRoadSegments();
		 BusStop* busStop_ptr = nullptr;
		 for(std::set<sim_mob::RoadSegment*>::const_iterator i = segments_.begin();i !=segments_.end();i++)
		 {
			sim_mob::BusStop* bustop_ = (*i)->getBusStop();
			busStop_ptr = getBusStop(node,(*i));
			if(busStop_ptr)
			{
			double newDist = sim_mob::dist(busStop_ptr->xPos, busStop_ptr->yPos,node->location.getX(), node->location.getY());
			if((newDist<dist || dist==0)&& busStop_ptr->BusLines.size()!=0)
			   {
			     dist=newDist;
				 bs1=busStop_ptr;
			   }
			}
		 }
	 }
	 else
	 {
		 Point2D point = node->location;
		 const StreetDirectory::LaneAndIndexPair lane_index =  StreetDirectory::instance().getLane(point);
		 if(lane_index.lane_)
		 {
			 sim_mob::Link* link_= lane_index.lane_->getRoadSegment()->getLink();
			 const sim_mob::Link* link_2 = StreetDirectory::instance().searchLink(link_->getEnd(),link_->getStart());
			 BusStop* busStop_ptr = nullptr;

			 std::vector<sim_mob::RoadSegment*> segments_ ;

			 if(link_)
			 {
				 segments_= const_cast<Link*>(link_)->getSegments();
				 for(std::vector<sim_mob::RoadSegment*>::const_iterator i = segments_.begin();i != segments_.end();i++)
				 {
					sim_mob::BusStop* bustop_ = (*i)->getBusStop();
					busStop_ptr = getBusStop(node,(*i));
					if(busStop_ptr)
					{
						double newDist = sim_mob::dist(busStop_ptr->xPos, busStop_ptr->yPos,point.getX(), point.getY());
						if((newDist<dist || dist==0)&& busStop_ptr->BusLines.size()!=0)
						 {
						 	dist=newDist;
						 	bs1=busStop_ptr;
						 }
					}
				 }
			 }

			 if(link_2)
			 {
				 segments_ = const_cast<Link*>(link_2)->getSegments();
				 for(std::vector<sim_mob::RoadSegment*>::const_iterator i = segments_.begin();i != segments_.end();i++)
				 {
					sim_mob::BusStop* bustop_ = (*i)->getBusStop();
					busStop_ptr = getBusStop(node,(*i));
					if(busStop_ptr)
					{
						double newDist = sim_mob::dist(busStop_ptr->xPos, busStop_ptr->yPos,point.getX(), point.getY());
						if((newDist<dist || dist==0)&& busStop_ptr->BusLines.size()!=0)
					    {
						   dist=newDist;
						   bs1=busStop_ptr;
						 }
					}
				 }
			 }
		 }

	 }

	 return bs1;
}

sim_mob::BusStop* sim_mob::BusStopFinder::getBusStop(const Node* node,sim_mob::RoadSegment* segment)
{
	 std::map<centimeter_t, const RoadItem*>::const_iterator ob_it;
	 const std::map<centimeter_t, const RoadItem*> & obstacles =segment->obstacles;
	 for (ob_it = obstacles.begin(); ob_it != obstacles.end(); ++ob_it) {
		RoadItem* ri = const_cast<RoadItem*>(ob_it->second);
		BusStop *bs = dynamic_cast<BusStop*>(ri);
		if (bs && ((segment->getStart() == node) || (segment->getEnd() == node) )) {
			return bs;
		}
	 }

	 return nullptr;
}

void sim_mob::aimsun::Loader::CreateIntersectionManagers(const sim_mob::RoadNetwork& roadNetwork)
{
	//Iterate through all the multi-nodes
	for (vector<sim_mob::MultiNode*>::const_iterator itIntersection = roadNetwork.nodes.begin(); itIntersection != roadNetwork.nodes.end(); itIntersection++)
	{		
		//Check if it has a traffic signal
		if(!StreetDirectory::instance().signalAt(**itIntersection))
		{
			//No traffic signal at the multi-node, so create an intersection manager
			IntersectionManager *intMgr = new IntersectionManager(sim_mob::ConfigManager::GetInstance().FullConfig().mutexStategy(), *itIntersection);
			
			//Add it to the map
			IntersectionManager::intManagers.insert(std::make_pair((*itIntersection)->getID(), intMgr));
		}
	}
}
