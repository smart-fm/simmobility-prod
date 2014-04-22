//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "Loader.hpp"

#include <iostream> // TODO: Remove this when debugging is done ~ Harish
#include <set>
#include <vector>
#include <cmath>
#include <algorithm>
#include <stdexcept>

//NOTE: CMake should put the correct -I flags in for SOCI; be aware that some distros hide it though.
#include <soci.h>
#include <soci-postgresql.h>
#include <boost/multi_index_container.hpp>

#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "conf/settings/DisableMPI.h"
#include "entities/AuraManager.hpp"
#include "entities/conflux/SegmentStats.hpp"

#include "entities/misc/BusTrip.hpp"
#include "geospatial/Point2D.hpp"
#include "geospatial/Node.hpp"
#include "geospatial/UniNode.hpp"
#include "geospatial/MultiNode.hpp"
#include "geospatial/Intersection.hpp"
#include "geospatial/Link.hpp"
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

#include "logging/Log.hpp"
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
#include "entities/misc/BusSchedule.hpp"
#include "entities/misc/PublicTransit.hpp"
#include "entities/misc/aimsun/TripChain.hpp"
#include "entities/misc/aimsun/SOCI_Converters.hpp"
#include "entities/profile/ProfileBuilder.hpp"
#include "entities/conflux/Conflux.hpp"
#include "entities/Person.hpp"
#include "entities/signal/Signal.hpp"
#include "entities/BusStopAgent.hpp"

#include "partitions/PartitionManager.hpp"
#include "partitions/BoundarySegment.hpp"

using namespace sim_mob::aimsun;
using sim_mob::DynamicVector;
using sim_mob::Point2D;
using std::vector;
using std::string;
using std::set;
using std::map;
using std::pair;
using std::multimap;



namespace {


class DatabaseLoader : private boost::noncopyable {
public:
	explicit DatabaseLoader(string const & connectionString);

	void LoadBasicAimsunObjects(map<string, string> const & storedProcedures);
//	// load path set data
//	void LoadSinglePathDB(std::map<std::string,sim_mob::SinglePath*>& pool,
//			std::map<std::string,sim_mob::SinglePath*>& waypoint_singlepathPool);
//	bool LoadSinglePathDBwithId(std::map<std::string,sim_mob::SinglePath*>& pool,
//			std::map<std::string,sim_mob::SinglePath*>& waypoint_singlepathPool,std::string& pathset_id);
////	void LoadPathPoolDB(std::vector<sim_mob::PathPoolDB>& pool);
//	void LoadPathSetDB(std::map<std::string,sim_mob::PathSet* >& pool);
//	bool LoadPathSetDBwithId(std::map<std::string,sim_mob::PathSet* >& pool,std::string& pathset_id);
	void LoadERP_Surcharge(std::map<std::string,std::vector<sim_mob::ERP_Surcharge*> >& pool);
	void LoadERP_Section(std::map<std::string,sim_mob::ERP_Section*>& erp_section_pool);
	void LoadERP_Gantry_Zone(std::map<std::string,sim_mob::ERP_Gantry_Zone*>& erp_gantry_zone_pool);
	void Loadlink_default_travel_time(std::map<std::string,std::vector<sim_mob::Link_travel_time*> >& pool);
	bool Loadlink_realtime_travel_time(std::string& table_name,
			std::map<std::string,std::vector<sim_mob::Link_travel_time*> >& pool);
	bool CreateTable(std::string& table_name);
	bool InsertData2TravelTimeTmpTable(std::string& table_name,sim_mob::Link_travel_time& data);
	bool InsertCSV2Table(std::string& table_name,std::string& csvFileName);
	bool TruncateTable(std::string& table_name);
	bool ExcuString(std::string& str);
	// save path set data
	void InsertSinglePath2DB(std::map<std::string,sim_mob::SinglePath*>& pathPool);
	bool LoadSinglePathDBwithId2(
				std::map<std::string,sim_mob::SinglePath*>& waypoint_singlepathPool,
				std::string& pathset_id,
				std::vector<sim_mob::SinglePath*>& spPool);
	bool LoadPathSetDBwithId(
			std::map<std::string,sim_mob::PathSet* >& pool,
			std::string& pathset_id);
	bool LoadOnePathSetDBwithId(std::string& pathset_id,sim_mob::PathSet& ps);
	void InsertPathSet2DB(std::map<std::string,sim_mob::PathSet* >& pathSetPool);

#ifndef SIMMOB_DISABLE_MPI
	void TransferBoundaryRoadSegment();
#endif

	void DecorateAndTranslateObjects();
	void PostProcessNetwork();

	//NOTE: Some of these are handled automatically under SaveSimMobilityNetwork, but should really be separated out.
	//      I've migrated saveTripChains into its own function in order to help with this.
	void SaveSimMobilityNetwork(sim_mob::RoadNetwork& res, std::map<std::string, std::vector<sim_mob::TripChainItem*> >& tcs);
	void saveTripChains(std::map<std::string, std::vector<sim_mob::TripChainItem*> >& tcs);
    void SaveBusSchedule(std::vector<sim_mob::BusSchedule*>& busschedule);

	map<int, Section> const & sections() const { return sections_; }
	const map<std::string, vector<const sim_mob::BusStop*> >& getRoute_BusStops() const { return route_BusStops; }
	const map<std::string, vector<const sim_mob::RoadSegment*> >& getRoute_RoadSegments() const { return route_RoadSegments; }

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

public:
	void LoadTripchains(const std::string& storedProc);

public:
	//New-style Loader functions can simply load data directly into the result vectors.
	void LoadPTBusDispatchFreq(const std::string& storedProc, std::vector<sim_mob::PT_bus_dispatch_freq>& pt_bus_dispatch_freq);
	void LoadPTBusRoutes(const std::string& storedProc, std::vector<sim_mob::PT_bus_routes>& pt_bus_routes, std::map<std::string, std::vector<const sim_mob::RoadSegment*> >& routeID_roadSegments);
	void LoadPTBusStops(const std::string& storedProc, std::vector<sim_mob::PT_bus_stops>& pt_bus_stops, std::map<std::string, std::vector<const sim_mob::BusStop*> >& routeID_busStops);
	void LoadBusSchedule(const std::string& storedProc, std::vector<sim_mob::BusSchedule*>& busschedule);

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
void DatabaseLoader::InsertSinglePath2DB(std::map<std::string,sim_mob::SinglePath*>& pathPool)
{
	for(std::map<std::string,sim_mob::SinglePath*>::iterator it=pathPool.begin();it!=pathPool.end();++it)
	{
		sim_mob::SinglePath* sp = (*it).second;
		if(sp->isNeedSave2DB)
		{
			sql_<<"insert into \"SinglePath\"(\"ID\", \"PATHSET_ID\",\"UTILITY\",\"PATHSIZE\",\"TRAVEL_COST\",\"SIGNAL_NUMBER\",\"RIGHT_TURN_NUMBER\",\"SCENARIO\",\"LENGTH\",\"TRAVEL_TIME\") "
					"values(:ID, :PATHSET_ID,:UTILITY,:PATHSIZE,:TRAVEL_COST,:SIGNAL_NUMBER,:RIGHT_TURN_NUMBER,:SCENARIO,:LENGTH,:TRAVEL_TIME)", soci::use(*sp);
		}
	}
}
bool DatabaseLoader::LoadSinglePathDBwithId2(
				std::map<std::string,sim_mob::SinglePath*>& waypoint_singlepathPool,
				std::string& pathset_id,
				std::vector<sim_mob::SinglePath*>& spPool)
{
	//Our SQL statement
	//	std::cout<<"LoadSinglePathDBwithId: "<<pathset_id<<std::endl;
	//	std:string s = "'\"aimsun-id\":\"54204\",_\"aimsun-id\":\"59032\",'";
	//	std::cout<<"LoadSinglePathDBwithId: "<<s<<std::endl;
		soci::rowset<sim_mob::SinglePath> rs = (sql_.prepare <<"select * from \"SinglePath\" where \"PATHSET_ID\" =" + pathset_id);
		int i=0;
		for (soci::rowset<sim_mob::SinglePath>::const_iterator it=rs.begin(); it!=rs.end(); ++it)  {
			sim_mob::SinglePath *s = new sim_mob::SinglePath(*it);
	//		pool.insert(std::make_pair(s->dbData->id,s));
	//		std::cout<<"LSPDBwithId: waypointset size "<<s->id.size()<<std::endl;
			waypoint_singlepathPool.insert(std::make_pair(s->id,s));
			//
			spPool.push_back(s);
	//		SinglePathDBPool.push_back(&*it);
	//		std::cout<<"LoadSinglePathDB:  "<<i<<std::endl;
			i++;
		}
		if (i==0)
		{
			std::cout<<"LSPDBwithId: "<<pathset_id<< "no data in db"<<std::endl;
			return false;
		}
		return true;
}
bool DatabaseLoader::LoadPathSetDBwithId(
		std::map<std::string,sim_mob::PathSet* >& pool,
		std::string& pathset_id)
{
	//Our SQL statement
	soci::rowset<sim_mob::PathSet> rs = (sql_.prepare <<"select * from \"PathSet\" where \"ID\" = " + pathset_id);
	int i=0;
	for (soci::rowset<sim_mob::PathSet>::const_iterator it=rs.begin(); it!=rs.end(); ++it)  {
		//
		sim_mob::PathSet* ps = new sim_mob::PathSet(*it);
		pool.insert(std::make_pair(ps->id,ps));
		//
//		PathSetDBPool.push_back(&*it);
//		std::cout<<"LoadPathSetDB:  "<<i<<std::endl;
		i++;
	}
	if(i==0)
	{
		std::cout<<"LPSetDBwithId: "<<pathset_id<<" no data in db"<<std::endl;
		return false;
	}
	else
	{
		return true;
	}
}
bool DatabaseLoader::LoadOnePathSetDBwithId(std::string& pathset_id,sim_mob::PathSet& ps)
{
	//Our SQL statement
	soci::rowset<sim_mob::PathSet> rs = (sql_.prepare <<"select * from \"PathSet\" where \"ID\" = " + pathset_id);
	int i=0;
	for (soci::rowset<sim_mob::PathSet>::const_iterator it=rs.begin(); it!=rs.end(); ++it)  {
		//
		ps = sim_mob::PathSet(*it);
		i=1;
		break;
	}
	if(i==0)
	{
		std::cout<<"LPSetDBwithId: "<<pathset_id<<" no data in db"<<std::endl;
		return false;
	}
	else
	{
		return true;
	}
}
void DatabaseLoader::InsertPathSet2DB(std::map<std::string,sim_mob::PathSet* >& pathSetPool)
{
	for(std::map<std::string,sim_mob::PathSet* >::iterator it=pathSetPool.begin();it!=pathSetPool.end();++it)
	{
		sim_mob::PathSet *ps = (*it).second;
		if(ps->isNeedSave2DB)
		{
			sql_<<"insert into \"PathSet\"(\"ID\", \"FROM_NODE_ID\", \"TO_NODE_ID\",\"SINGLEPATH_ID\",\"SCENARIO\",\"HAS_PATH\") "
								   "values(:ID, :FROM_NODE_ID, :TO_NODE_ID,:SINGLEPATH_ID,:SCENARIO,:HAS_PATH)", soci::use(*ps);
		}
	}
}

//void DatabaseLoader::LoadPathPoolDB(std::vector<sim_mob::PathPoolDB>& pool)
//{
//	//Our SQL statement
//	soci::rowset<sim_mob::PathPoolDB> rs = (sql_.prepare <<"select * from \"PathPoolDB\" ");
//	for (soci::rowset<sim_mob::PathPoolDB>::const_iterator it=rs.begin(); it!=rs.end(); ++it)  {
//		pool.push_back(*it);
//	}
//}
void DatabaseLoader::Loadlink_default_travel_time(std::map<std::string,
		std::vector<sim_mob::Link_travel_time*> >& pool)
{
	soci::rowset<sim_mob::Link_travel_time> rs = (sql_.prepare <<"select \"link_id\",to_char(\"start_time\",'HH24:MI:SS') AS start_time,to_char(\"end_time\",'HH24:MI:SS') AS end_time,\"travel_time\" from \"link_default_travel_time\" ");
	for (soci::rowset<sim_mob::Link_travel_time>::const_iterator it=rs.begin(); it!=rs.end(); ++it)  {
		sim_mob::Link_travel_time *s = new sim_mob::Link_travel_time(*it);
//		std::cout<<"Link_travel_time: "<<s->start_time<<std::endl;
		std::map<std::string,std::vector<sim_mob::Link_travel_time*> >::iterator itt = pool.find(s->originalSectionDB_ID.getLogItem());
		if(itt!=pool.end())
		{
			std::vector<sim_mob::Link_travel_time*> e = (*itt).second;
			e.push_back(s);
			pool[s->originalSectionDB_ID.getLogItem()] = e;
//			insert(std::make_pair(s->originalSectionDB_ID.getLogItem(),s));
		}
		else
		{
			std::vector<sim_mob::Link_travel_time*> e;
			e.push_back(s);
			pool[s->originalSectionDB_ID.getLogItem()] = e;
		}
	}
}
bool DatabaseLoader::Loadlink_realtime_travel_time(std::string& table_name,
		std::map<std::string,std::vector<sim_mob::Link_travel_time*> >& pool)
{
	try {
		soci::rowset<sim_mob::Link_travel_time> rs = (sql_.prepare <<"select \"link_id\",to_char(\"start_time\",'HH24:MI:SS') AS start_time,to_char(\"end_time\",'HH24:MI:SS') AS end_time,\"travel_time\" from " + table_name);
		for (soci::rowset<sim_mob::Link_travel_time>::const_iterator it=rs.begin(); it!=rs.end(); ++it)  {
			sim_mob::Link_travel_time *s = new sim_mob::Link_travel_time(*it);
	//		std::cout<<"Link_travel_time: "<<s->start_time<<std::endl;
			std::map<std::string,std::vector<sim_mob::Link_travel_time*> >::iterator itt = pool.find(s->originalSectionDB_ID.getLogItem());
			if(itt!=pool.end())
			{
				std::vector<sim_mob::Link_travel_time*> e = (*itt).second;
				e.push_back(s);
				pool[s->originalSectionDB_ID.getLogItem()] = e;
	//			insert(std::make_pair(s->originalSectionDB_ID.getLogItem(),s));
			}
			else
			{
				std::vector<sim_mob::Link_travel_time*> e;
				e.push_back(s);
				pool[s->originalSectionDB_ID.getLogItem()] = e;
			}
		}
		return true;
	}
	catch (soci::soci_error const & err)
	{
		std::cout<<"Loadlink_realtime_travel_time: "<<err.what()<<std::endl;
		return false;
	}
}
bool DatabaseLoader::CreateTable(std::string& table_name)
{
	try {
//		sql_  << ("CREATE TABLE \"max_12345\" ( \"link_id\" integer NOT NULL,\"start_time\" time without time zone NOT NULL,\"end_time\" time without time zone NOT NULL,\"travel_time\" double precision )");
//		sql_  << ("CREATE TABLE "+table_name+" ( \"link_id\" integer NOT NULL,\"start_time\" time without time zone NOT NULL,\"end_time\" time without time zone NOT NULL,\"travel_time\" double precision )");
		sql_  << ("CREATE TABLE "+table_name);
		sql_.commit();
	}
	catch (soci::soci_error const & err)
	{
		std::cout<<"CreateTable: "<<err.what()<<std::endl;
		return false;
	}
	std::cout<<"CreateTable: create table "<<table_name<<" ok"<<std::endl;
	return true;
}
bool DatabaseLoader::InsertData2TravelTimeTmpTable(std::string& table_name,
		sim_mob::Link_travel_time& data)
{
	try {
		sql_<<"insert into "+ table_name +" (\"link_id\", \"start_time\",\"end_time\",\"travel_time\") "
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
bool DatabaseLoader::InsertCSV2Table(std::string& table_name,std::string& csvFileName)
{
	try {
		sql_ << ("COPY " + table_name + " FROM '" + csvFileName + "' WITH DELIMITER AS ';'");
		sql_.commit();
		}
		catch (soci::soci_error const & err)
		{
			std::cout<<"InsertCSV2Table: "<<err.what()<<std::endl;
			return false;
		}
		return true;
}
bool DatabaseLoader::TruncateTable(std::string& table_name)
{
	try {
		sql_<<"TRUNCATE TABLE "+ table_name;
		sql_.commit();
	}
	catch (soci::soci_error const & err)
	{
		std::cout<<"TruncateTable: "<<err.what()<<std::endl;
		return false;
	}
	return true;
}
bool DatabaseLoader::ExcuString(std::string& str)
{
	try {
		sql_<<str;
		sql_.commit();
	}
	catch (soci::soci_error const & err)
	{
		std::cout<<"ExcuString: "<<err.what()<<std::endl;
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
//		std::cout<<"LoadERP_Surcharge: "<<s.Start_Time<<std::endl;
		std::map<std::string,std::vector<sim_mob::ERP_Surcharge*> >::iterator itt = pool.find(s->Gantry_No);
		if(itt!=pool.end())
		{
			std::vector<sim_mob::ERP_Surcharge*> e = (*itt).second;
			e.push_back(s);
			pool[s->Gantry_No] = e;
		}
		else
		{
			std::vector<sim_mob::ERP_Surcharge*> e;
			e.push_back(s);
			pool[s->Gantry_No] = e;
		}
	}
}
void DatabaseLoader::LoadERP_Section(std::map<std::string,sim_mob::ERP_Section*>& erp_section_pool)
{
	soci::rowset<sim_mob::ERP_Section> rs = (sql_.prepare <<"select * from \"ERP_Section\" ");
	for (soci::rowset<sim_mob::ERP_Section>::const_iterator it=rs.begin(); it!=rs.end(); ++it)  {
		sim_mob::ERP_Section *s = new sim_mob::ERP_Section(*it);
//		erp_section_pool.insert(std::make_pair(s->ERP_Gantry_No,s));
		erp_section_pool.insert(std::make_pair(s->originalSectionDB_ID.getLogItem(),s));
//		std::cout<<"LoadERP_Section: "<<s->originalSectionDB_ID.getLogItem()<<std::endl;
//		std::cout<<"LoadERP_Section: "<<s->ERP_Gantry_No<<" "<<s->section_id<<std::endl;
	}
}
void DatabaseLoader::LoadERP_Gantry_Zone(std::map<std::string,sim_mob::ERP_Gantry_Zone*>& erp_gantry_zone_pool)
{
	soci::rowset<sim_mob::ERP_Gantry_Zone> rs = (sql_.prepare <<"select * from \"ERP_Gantry_Zone\" ");
	for (soci::rowset<sim_mob::ERP_Gantry_Zone>::const_iterator it=rs.begin(); it!=rs.end(); ++it)  {
		sim_mob::ERP_Gantry_Zone *s = new sim_mob::ERP_Gantry_Zone(*it);
		erp_gantry_zone_pool.insert(std::make_pair(s->Gantry_no,s));
//		std::cout<<"LoadERP_Section: "<<s->Gantry_no<<" "<<s->Zone_Id<<std::endl;
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

//		//Convert meters to cm
//		it->xPos *= 100;
//		it->yPos *= 100;

		nodes_[it->id] = *it;
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
			std::cout <<"From node: " <<it->TMP_FromNodeID  <<"  " <<nodes_.count(it->TMP_FromNodeID) <<"\n";
			std::cout <<"To node: " <<it->TMP_ToNodeID  <<"  " <<nodes_.count(it->TMP_ToNodeID) <<"\n";
			throw std::runtime_error("Invalid From or To node.");
		}

//		//Convert meters to cm
//		it->length *= 100;

		//Note: Make sure not to resize the Node map after referencing its elements.
		it->fromNode = &nodes_[it->TMP_FromNodeID];
		it->toNode = &nodes_[it->TMP_ToNodeID];

		sections_[it->id] = *it;
	}
}

void DatabaseLoader::LoadPhase(const std::string& storedProc)
{
	//Optional
	if (storedProc.empty()) {
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

//		//Convert meters to cm
//		it->xPos *= 100;
//		it->yPos *= 100;

		//Note: Make sure not to resize the Section vector after referencing its elements.
		it->atSection = &sections_[it->TMP_AtSectionID];
		crossings_.push_back(*it);
	}
}

void DatabaseLoader::LoadLanes(const std::string& storedProc)
{
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

//		//Convert meters to cm
//		it->xPos *= 100;
//		it->yPos *= 100;

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
 * (Turning contains four columns four columns pertaining to lanes)
 * vahid
 */
void DatabaseLoader::LoadTurnings(const std::string& storedProc)
{
	//Our SQL statement
	soci::rowset<Turning> rs = (sql_.prepare <<"select * from " + storedProc);

	//Exectue as a rowset to avoid repeatedly building the query.
	vector<int> skippedTurningIDs;
	turnings_.clear();
	for (soci::rowset<Turning>::const_iterator it=rs.begin(); it!=rs.end(); ++it)  {
		//Check nodes
		bool fromMissing = sections_.count(it->TMP_FromSection)==0;
		bool toMissing = sections_.count(it->TMP_ToSection)==0;
		if(fromMissing || toMissing) {
			skippedTurningIDs.push_back(it->id);
			continue;
		}

		//Note: Make sure not to resize the Section map after referencing its elements.
		it->fromSection = &sections_[it->TMP_FromSection];
		it->toSection = &sections_[it->TMP_ToSection];
		turnings_[it->id] = *it;
	}

	//Print skipped turnings all at once.
//	sim_mob::PrintArray(skippedTurningIDs, std::cout, "Turnings skipped: ", "[", "]", ", ", 4);
}

void DatabaseLoader::LoadPolylines(const std::string& storedProc)
{
	//Our SQL statement
	soci::rowset<Polyline> rs = (sql_.prepare <<"select * from " + storedProc);

	//Exectue as a rowset to avoid repeatedly building the query.
	polylines_.clear();
	for (soci::rowset<Polyline>::const_iterator it=rs.begin(); it!=rs.end(); ++it)  {
		//Check nodes
		if(sections_.count(it->TMP_SectionId)==0) {
			throw std::runtime_error("Invalid polyline section reference.");
		}

//		//Convert meters to cm
//		it->xPos *= 100;
//		it->yPos *= 100;

		//Note: Make sure not to resize the Section map after referencing its elements.
		it->section = &sections_[it->TMP_SectionId];
		polylines_.insert(std::make_pair(it->section->id, *it));
		//polylines_[it->id] = *it;
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
				tripchains_.push_back(*it);
				std::cout << "from stop: " << it->tmp_fromLocationNodeID << " to stop: " << it->tmp_toLocationNodeID << std::endl;
			}
			//check nodes
			if(nodes_.count(it->tmp_fromLocationNodeID)==0) {
				std::cout<< "Invalid trip chain fromNode reference."<<std::endl;
				//throw std::runtime_error("Invalid trip chain fromNode reference.");
				continue;
			}
			if(nodes_.count(it->tmp_toLocationNodeID)==0) {
				std::cout<< "Invalid trip chain toNode reference."<<std::endl;
				//throw std::runtime_error("Invalid trip chain toNode reference.");
				continue;
			}

			//Note: Make sure not to resize the Node map after referencing its elements.
			it->fromLocation = &nodes_[it->tmp_fromLocationNodeID];
			it->toLocation = &nodes_[it->tmp_toLocationNodeID];
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
//        // Convert from meters to centimeters.
//        iter->xPos *= 100;
//        iter->yPos *= 100;
        signals_.insert(std::make_pair(iter->id, *iter));
    }
}

void DatabaseLoader::LoadBusStop(const std::string& storedProc)
{
	//Bus stops are optional
	if (storedProc.empty()) {
		return;
	}

	soci::rowset<BusStop> rows = (sql_.prepare <<"select * from " + storedProc);
	for (soci::rowset<BusStop>::const_iterator iter = rows.begin(); iter != rows.end(); ++iter)
	{
		BusStop busstop = *iter;
//		         Convert from meters to centimeters.
//		        busstop.xPos *= 100;
//		        busstop.yPos *= 100;
	        busstop_.insert(std::make_pair(busstop.bus_stop_no, busstop));
		        //std :: cout.precision(15);
		        //std :: cout << "Bus Stop ID is: "<< busstop.bus_stop_no <<"    "<< busstop.xPos << "     "<< busstop.yPos  <<std::endl;

		        //it->atSection = &sections_[it->TMP_AtSectionID];
		        	//	busstop_.push_back(*it);
	}
}

void DatabaseLoader::LoadBusStopSG(const std::string& storedProc)
{
	//Bus stops are optional
	if (storedProc.empty()) {
		return;
	}

	soci::rowset<BusStopSG> rows = (sql_.prepare <<"select * from " + storedProc);
	for (soci::rowset<BusStopSG>::const_iterator iter = rows.begin(); iter != rows.end(); ++iter)
	{
		BusStopSG busstop = *iter;
//		         Convert from meters to centimeters.
		busstop.bus_stop_no.erase(remove_if(busstop.bus_stop_no.begin(), busstop.bus_stop_no.end(), isspace),
				busstop.bus_stop_no.end());
		busstop.stop_lat.erase(remove_if(busstop.stop_lat.begin(), busstop.stop_lat.end(), isspace),
				busstop.stop_lat.end());
		busstop.stop_lon.erase(remove_if(busstop.stop_lon.begin(), busstop.stop_lon.end(), isspace),
				busstop.stop_lon.end());

//		        busstop.xPos = boost::lexical_cast<double>(busstop.stop_lat) * 100;
//		        busstop.yPos = boost::lexical_cast<double>(busstop.stop_lon) * 100;
        busstop.xPos = boost::lexical_cast<double>(busstop.stop_lat);
        busstop.yPos = boost::lexical_cast<double>(busstop.stop_lon);
		        bustopSG_.insert(std::make_pair(busstop.bus_stop_no, busstop));
		        //std :: cout.precision(15);
		        //std :: cout << "Bus Stop ID is: "<< busstop.bus_stop_no <<"    "<< busstop.xPos << "     "<< busstop.yPos  <<std::endl;

		        //it->atSection = &sections_[it->TMP_AtSectionID];
		        	//	busstop_.push_back(*it);
	}
}

void DatabaseLoader::LoadPTBusDispatchFreq(const std::string& storedProc, std::vector<sim_mob::PT_bus_dispatch_freq>& pt_bus_dispatch_freq)
{
	if (storedProc.empty())
	{
		sim_mob::Warn() << "WARNING: An empty 'PT_bus_dispatch_freq' stored-procedure was specified in the config file; " << std::endl;
		return;
	}
	soci::rowset<sim_mob::PT_bus_dispatch_freq> rows = (sql_.prepare <<"select * from " + storedProc);
	for (soci::rowset<sim_mob::PT_bus_dispatch_freq>::const_iterator iter = rows.begin(); iter != rows.end(); ++iter)
	{
		//sim_mob::PT_bus_dispatch_freq* pt_bus_freqTemp = new sim_mob::PT_bus_dispatch_freq(*iter);
		sim_mob::PT_bus_dispatch_freq pt_bus_freqTemp = *iter;
		pt_bus_freqTemp.route_id.erase(remove_if(pt_bus_freqTemp.route_id.begin(), pt_bus_freqTemp.route_id.end(), isspace),
				pt_bus_freqTemp.route_id.end());
		pt_bus_freqTemp.frequency_id.erase(remove_if(pt_bus_freqTemp.frequency_id.begin(), pt_bus_freqTemp.frequency_id.end(), isspace),
				pt_bus_freqTemp.frequency_id.end());
		pt_bus_dispatch_freq.push_back(pt_bus_freqTemp);
		std::cout << pt_bus_freqTemp.frequency_id << " " << pt_bus_freqTemp.route_id << " " << pt_bus_freqTemp.headway_sec << " " << pt_bus_freqTemp.start_time.toString() << std::endl;
	}
}

void DatabaseLoader::LoadPTBusRoutes(const std::string& storedProc, std::vector<sim_mob::PT_bus_routes>& pt_bus_routes, std::map<std::string, std::vector<const sim_mob::RoadSegment*> >& routeID_roadSegments)
{
	if (storedProc.empty())
	{
		sim_mob::Warn() << "WARNING: An empty 'pt_bus_routes' stored-procedure was specified in the config file; " << std::endl;
		return;
	}
	soci::rowset<sim_mob::PT_bus_routes> rows = (sql_.prepare <<"select * from " + storedProc);
	for (soci::rowset<sim_mob::PT_bus_routes>::const_iterator iter = rows.begin(); iter != rows.end(); ++iter)
	{
		sim_mob::PT_bus_routes pt_bus_routesTemp = *iter;
		pt_bus_routes.push_back(pt_bus_routesTemp);
//		std::cout << pt_bus_routesTemp.route_id << " " << atoi(pt_bus_routesTemp.link_id.c_str()) << " " << pt_bus_routesTemp.link_sequence_no << std::endl;
		sim_mob::RoadSegment *seg = sections_[atoi(pt_bus_routesTemp.link_id.c_str())].generatedSegment;
		if(seg) {
			routeID_roadSegments[iter->route_id].push_back(seg);
//			std::cout << "iter->route_id: " << iter->route_id << "    Section to segment map  " << seg->getSegmentID() ;
//			std::cout << "current routeID_roadSegments[iter->route_id].size(): " << routeID_roadSegments[iter->route_id].size() << "" << std::endl;
		}
	}
//	std::cout << "routeID_roadSegments.size(): " << routeID_roadSegments.size() << "" << std::endl;
}

void DatabaseLoader::LoadPTBusStops(const std::string& storedProc, std::vector<sim_mob::PT_bus_stops>& pt_bus_stops, std::map<std::string, std::vector<const sim_mob::BusStop*> >& routeID_busStops)
{
	sim_mob::ConfigParams& config = sim_mob::ConfigManager::GetInstanceRW().FullConfig();
	if (storedProc.empty())
	{
		sim_mob::Warn() << "WARNING: An empty 'pt_bus_stops' stored-procedure was specified in the config file; " << std::endl;
		return;
	}
	soci::rowset<sim_mob::PT_bus_stops> rows = (sql_.prepare <<"select * from " + storedProc);
	for (soci::rowset<sim_mob::PT_bus_stops>::const_iterator iter = rows.begin(); iter != rows.end(); ++iter)
	{
		sim_mob::PT_bus_stops pt_bus_stopsTemp = *iter;
		pt_bus_stops.push_back(pt_bus_stopsTemp);
//		std::cout << pt_bus_stopsTemp.route_id << " " << pt_bus_stopsTemp.busstop_no << " " << pt_bus_stopsTemp.busstop_sequence_no << std::endl;
		sim_mob::BusStop* bs = config.getBusStopNo_BusStops()[pt_bus_stopsTemp.busstop_no];
		if(bs) {
			routeID_busStops[iter->route_id].push_back(bs);
//			std::cout << "iter->route_id: " << iter->route_id << "    BusStop to busstop map  " << bs->getBusstopno_() << "" << std::endl;
//			std::cout << "current routeID_busStops[iter->route_id].size(): " << routeID_busStops[iter->route_id].size() << "" << std::endl;
		}
	}
//	std::cout << "routeID_busStops.size(): " << routeID_busStops.size() << "" << std::endl;
}

void DatabaseLoader::LoadBusSchedule(const std::string& storedProc, std::vector<sim_mob::BusSchedule*>& busschedule)
{
    if (storedProc.empty()) {
    	sim_mob::Warn() << "WARNING: An empty 'bus_schedule' stored-procedure was specified in the config file; "
               << "will not lookup the database to create any signal found in there" << std::endl;
        return;
    }
    soci::rowset<sim_mob::BusSchedule> rows = (sql_.prepare <<"select * from " + storedProc);
    for (soci::rowset<sim_mob::BusSchedule>::const_iterator iter = rows.begin(); iter != rows.end(); ++iter)
    {
    	busschedule.push_back(new sim_mob::BusSchedule(*iter));
    }
}




std::string getStoredProcedure(map<string, string> const & storedProcs, string const & procedureName, bool mandatory=true)
{
	map<string, string>::const_iterator iter = storedProcs.find(procedureName);
	if (iter != storedProcs.end())
		return iter->second;
	if (!mandatory) {
		std::cout <<"Skipping optional database property: " + procedureName <<std::endl;
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

void DatabaseLoader::LoadBasicAimsunObjects(map<string, string> const & storedProcs)
{
	LoadNodes(getStoredProcedure(storedProcs, "node"));
	LoadSections(getStoredProcedure(storedProcs, "section"));
	LoadCrossings(getStoredProcedure(storedProcs, "crossing"));
	LoadLanes(getStoredProcedure(storedProcs, "lane"));
	LoadTurnings(getStoredProcedure(storedProcs, "turning"));
	LoadPolylines(getStoredProcedure(storedProcs, "polyline"));
	LoadTripchains(getStoredProcedure(storedProcs, "tripchain", false));
	LoadTrafficSignals(getStoredProcedure(storedProcs, "signal", false));
	LoadBusStop(getStoredProcedure(storedProcs, "busstop", false));
	LoadBusStopSG(getStoredProcedure(storedProcs, "busstopSG", false));
	LoadPhase(getStoredProcedure(storedProcs, "phase"));

	//add by xuyan
	//load in boundary segments (not finished!)
#ifndef SIMMOB_DISABLE_MPI
	const sim_mob::ConfigParams& config = sim_mob::ConfigManager::GetInstance().FullConfig();
	if (config.using_MPI) {
		LoadBoundarySegments();
	}
#endif

}



//Compute the distance from the source node of the polyline to a
// point on the line from the source to the destination nodes which
// is normal to the Poly-point.
void ComputePolypointDistance(Polyline& pt)
{
	//Our method is (fairly) simple.
	//First, compute the distance from the point to the polyline at a perpendicular angle.
	double dx2x1 = pt.section->toNode->xPos - pt.section->fromNode->xPos;
	double dy2y1 = pt.section->toNode->yPos - pt.section->fromNode->yPos;
	double dx1x0 = pt.section->fromNode->xPos - pt.xPos;
	double dy1y0 = pt.section->fromNode->yPos - pt.yPos;
	double numerator = dx2x1*dy1y0 - dx1x0*dy2y1;
	double denominator = sqrt(dx2x1*dx2x1 + dy2y1*dy2y1);
	double perpenDist = numerator/denominator;
	if (perpenDist<0.0) {
		//We simplify all the quadratic math to just a sign change, since
		//   it's known that this polypoint has a positive distance to the line.
		perpenDist *= -1;
	}

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
	for (map<int,Section>::iterator it=sections_.begin(); it!=sections_.end(); it++) {
		//		if(it->second.fromNode) it->second.fromNode->sectionsAtNode.push_back(&(it->second));
		//		if(it->second.toNode) it->second.toNode->sectionsAtNode.push_back(&(it->second));
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
	for (map<int,Node>::iterator it=nodes_.begin(); it!=nodes_.end(); it++) {
		Node* n = &it->second;
		n->candidateForSegmentNode = true; //Conditional pass

		//Perform both checks at the same time.
		pair<Node*, Node*> others(nullptr, nullptr);
		pair<unsigned int, unsigned int> flags(0, 0);  //1="from->to", 2="to->from"
		string expectedName;
		for (vector<Section*>::iterator it=n->sectionsAtNode.begin(); it!=n->sectionsAtNode.end(); it++) {
			//Get "other" node
			Node* otherNode = ((*it)->fromNode!=n) ? (*it)->fromNode : (*it)->toNode;

			//Manage property one.
			unsigned int* flagPtr;
			if (!others.first || others.first==otherNode) {
				others.first = otherNode;
				flagPtr = &flags.first;
			} else if (!others.second || others.second==otherNode) {
				others.second = otherNode;
				flagPtr = &flags.second;
			} else {
				n->candidateForSegmentNode = false; //Fail
				break;
			}

			//Manage property two.
			unsigned int toFlag = ((*it)->toNode==n) ? 1 : 2;
			if (((*flagPtr)&toFlag)==0) {
				*flagPtr = (*flagPtr) | toFlag;
			} else {
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
	}
	//Next, Links and RoadSegments. See comments for our approach.
	for (map<int,Section>::iterator it=sections_.begin(); it!=sections_.end(); it++) {
		if (!it->second.hasBeenSaved) {  //Workaround...
			sim_mob::aimsun::Loader::ProcessSection(res, it->second);
		}
	}
	//Scan the vector to see if any skipped Sections were not filled in later.
	for (map<int,Section>::iterator it=sections_.begin(); it!=sections_.end(); it++) {
		if (!it->second.hasBeenSaved) {
			throw std::runtime_error("Section was skipped.");
		}
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
//	for (map<int,Node>::iterator it=nodes_.begin(); it!=nodes_.end(); it++) {
//		Node origin = (*it).second;
//		for (vector<Section*>::iterator it_sec=origin.sectionsAtNode.begin(); it_sec!=origin.sectionsAtNode.end(); it_sec++) {
//			sim_mob::RoadSegment ** it_seg = &((*it_sec)->generatedSegment);
//			if(!(((*it_seg)->getSegmentID() == 100001005)||((*it_seg)->getSegmentID() == 100001004))) continue;
//			for(std::map<sim_mob::centimeter_t, const sim_mob::RoadItem*>::iterator it_obs = (*it_seg)->obstacles.begin(); it_obs != (*it_seg)->obstacles.end(); it_obs++)
//					{
//						const sim_mob::Crossing * cr = dynamic_cast<const sim_mob::Crossing *>((*it_obs).second);
//						if((cr))
//						{
//							std::cout << "SaveSimMobilityNetwork::Segment " << (*it_seg) << "  " << (*it_seg)->getSegmentID() << " has crossing = " << cr->getCrossingID() <<  "  " << cr << " BTW: obstacle size = " << (*it_seg)->obstacles.size() << std::endl;
//						}
//					}
//		}
//	}
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
    //std::set<sim_mob::Node const *> uniNodes;
    std::set<sim_mob::Node const *> badNodes;
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
//            throw std::runtime_error(stream.str());
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
//    		std::cout << "There is no phase associated with this signal candidate("<< node->getID() <<"), bypassing\n";
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
    std::cout << "A Total of " << nof_signals << " were successfully created\n";
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
//		std::cout << " Signal(" << sid << ") : Number of Phases : " << nof_phases << std::endl;
		if(nof_phases > 0)
			if((nof_phases > 5)||(nof_phases < 1))
				std::cout << sid << " ignored due to lack of default choice set" << nof_phases ;
			else
			{
				plan.setDefaultSplitPlan(nof_phases);//i hope the nof phases is within the range of 2-5
			}
		else
			std::cout << sid << " ignored due to no phases" << nof_phases <<  std::endl;
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
	//int j = 0；
	//Save all bus stops
	for(map<std::string,BusStop>::iterator it = busstop_.begin(); it != busstop_.end(); it++) {
		std::map<int,Section>::iterator findPtr = sections_.find(it->second.TMP_AtSectionID);
		if(findPtr == sections_.end())
		{
			continue;
		}
		//Create the bus stop
		sim_mob::BusStop *busstop = new sim_mob::BusStop();
		sim_mob::RoadSegment* parentSeg = sections_[it->second.TMP_AtSectionID].generatedSegment;
		busstop->busstopno_ = it->second.bus_stop_no;
		busstop->setParentSegment(parentSeg);

		busstop->xPos = it->second.xPos;
		busstop->yPos = it->second.yPos;

		//Add the bus stop to its parent segment's obstacle list at an estimated offset.
		double distOrigin = sim_mob::BusStop::EstimateStopPoint(busstop->xPos, busstop->yPos, sections_[it->second.TMP_AtSectionID].generatedSegment);
		if(!busstop->getParentSegment()->addObstacle(distOrigin, busstop)) {
			sim_mob::Warn() << "Can't add obstacle; something is already at that offset. " << busstop->busstopno_ << std::endl;
		}

		sim_mob::ConfigManager::GetInstanceRW().FullConfig().getBusStopNo_BusStops()[busstop->busstopno_] = busstop;

		//set obstacle ID only after adding it to obstacle list. For Now, it is how it works. sorry
		busstop->setRoadItemID(sim_mob::BusStop::generateRoadItemID(*(busstop->getParentSegment())));//sorry this shouldn't be soooo explicitly set/specified, but what to do, we don't have parent segment when we were creating the busstop. perhaps a constructor argument!?  :) vahid
		sim_mob::BusStopAgent::RegisterNewBusStopAgent(*busstop, sim_mob::ConfigManager::GetInstance().FullConfig().mutexStategy());
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
		sim_mob::BusStopAgent::RegisterNewBusStopAgent(*busstop, sim_mob::ConfigManager::GetInstance().FullConfig().mutexStategy());
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
					if(shift.getX() > 1000)
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
	src.hasBeenSaved = true;

	sim_mob::Node* newNode = nullptr;
	if (!src.candidateForSegmentNode) {
		//This is an Intersection
		newNode = new sim_mob::Intersection(src.getXPosAsInt(), src.getYPosAsInt());

		//Store it in the global nodes array
		res.nodes.push_back(dynamic_cast<MultiNode*>(newNode));
	} else {
		//Just save for later so the pointer isn't invalid
		newNode = new UniNode(src.getXPosAsInt(), src.getYPosAsInt());
		res.segmentnodes.insert(dynamic_cast<UniNode*>(newNode));
	}

	//vahid
	newNode->setID(src.id);
	//For future reference
	src.generatedNode = newNode;
}


void sim_mob::aimsun::Loader::ProcessUniNode(sim_mob::RoadNetwork& res, Node& src)
{
	//Find 2 sections "from" and 2 sections "to".
	//(Bi-directional segments will complicate this eventually)
	//Most of the checks done here are already done earlier in the Loading process, but it doesn't hurt to check again.
	pair<Section*, Section*> fromSecs(nullptr, nullptr);
	pair<Section*, Section*> toSecs(nullptr, nullptr);
	for (vector<Section*>::iterator it=src.sectionsAtNode.begin(); it!=src.sectionsAtNode.end(); it++) {
		if ((*it)->TMP_ToNodeID==src.id) {
			if (!fromSecs.first) {
				fromSecs.first = *it;
			} else if (!fromSecs.second) {
				fromSecs.second = *it;
			} else {
				throw std::runtime_error("UniNode contains unexpected additional Sections leading TO.");
			}
		} else if ((*it)->TMP_FromNodeID==src.id) {
			if (!toSecs.first) {
				toSecs.first = *it;
			} else if (!toSecs.second) {
				toSecs.second = *it;
			} else {
				throw std::runtime_error("UniNode contains unexpected additional Sections leading FROM.");
			}
		} else {
			throw std::runtime_error("UniNode contains a Section which actually does not lead to/from that Node.");
		}
	}

	//Ensure at least one path was found, and a non-partial second path.
	if (!(fromSecs.first && toSecs.first)) {
		throw std::runtime_error("UniNode contains no primary path.");
	}
	if ((fromSecs.second && !toSecs.second) || (!fromSecs.second && toSecs.second)) {
		throw std::runtime_error("UniNode contains partial secondary path.");
	}

	//This is a simple Road Segment joint
	UniNode* newNode = dynamic_cast<UniNode*>(src.generatedNode);
	//newNode->location = new Point2D(src.getXPosAsInt(), src.getYPosAsInt());

	//Set locations (ensure unset locations are null)
	//Also ensure that we don't point backwards from the same segment.
	bool parallel = fromSecs.first->fromNode->id == toSecs.first->toNode->id;
	newNode->firstPair.first = fromSecs.first->generatedSegment;
	newNode->firstPair.second = parallel ? toSecs.second->generatedSegment : toSecs.first->generatedSegment;
	if (fromSecs.second && toSecs.second) {
		newNode->secondPair.first = fromSecs.second->generatedSegment;
		newNode->secondPair.second = parallel ? toSecs.first->generatedSegment : toSecs.second->generatedSegment;
	} else {
		newNode->secondPair = pair<RoadSegment*, RoadSegment*>(nullptr, nullptr);
	}

	//Save it for later reference
	//res.segmentnodes.insert(newNode);

	//TODO: Actual connector alignment (requires map checking)
	sim_mob::UniNode::buildConnectorsFromAlignedLanes(newNode, std::make_pair(0, 0), std::make_pair(0, 0));
////	if(newNode->getID() == 92370)
//	{
//		std::cout << "UniNode " <<   newNode->getID() << " has " << newNode->getConnectors().size() << " Connectors\n";
//	}

	//This UniNode can later be accessed by the RoadSegment itself.
}

sim_mob::RoadSegment * createNewRoadSegment(sim_mob::Link* ln, size_t numExistingSegsInLink, unsigned long id)
{
	return new sim_mob::RoadSegment(ln, nullptr, ln->getLinkId()*100 +numExistingSegsInLink);
}


void sim_mob::aimsun::Loader::ProcessSection(sim_mob::RoadNetwork& res, Section& src)
{
	//Skip Sections which start at a non-intersection. These will be filled in later.
	if (src.fromNode->candidateForSegmentNode) {
		return;
	}
	set<RoadSegment*> linkSegments;
	//std::ostringstream convertLinkId,convertSegId;

	//Process this section, and continue processing Sections along the direction of
	// travel until one of these ends on an intersection.
	//NOTE: This approach is far from foolproof; for example, if a Link contains single-directional
	//      Road segments that fail to match at every UniNode. Need to find a better way to
	//      group RoadSegments into Links, but at least this works for our test network.
	Section* currSec = &src;  //Which section are we currently processing?
	sim_mob::Link* ln = new sim_mob::Link(1000001 + res.links.size());//max ten million links
	src.generatedSegment = createNewRoadSegment(ln,linkSegments.size(),src.id);
	ln->roadName = currSec->roadName;
	ln->start = currSec->fromNode->generatedNode;

	//Make sure the link's start node is represented at the Node level.
	//TODO: Try to avoid dynamic casting if possible.
	for (;;) {
		//Update
		ln->end = currSec->toNode->generatedNode;

		//Check: not processing an existing segment
		if (currSec->hasBeenSaved) {
			throw std::runtime_error("Section processed twice.");
		}

		//Mark saved
		currSec->hasBeenSaved = true;

		//Check name
		if (ln->roadName != currSec->roadName) {
			throw std::runtime_error("Road names don't match up on RoadSegments in the same Link.");
		}

		//Prepare a new segment IF required, and save it for later reference (or load from past ref.)
		if (!currSec->generatedSegment) {
			//convertSegId.clear();
			//convertSegId.str(std::string());
			currSec->generatedSegment = createNewRoadSegment(ln,linkSegments.size(),src.id);
		} else {
//			std::cout << "Bypassing\n";
		}

		//Save this segment if either end points are multinodes
		//TODO: This should be done at a global level, once the network has been loaded (similar to how XML does it).
		for (size_t tempID=0; tempID<2; tempID++) {
			sim_mob::Node* nd = tempID==0?currSec->fromNode->generatedNode:currSec->toNode->generatedNode;
			sim_mob::MultiNode* multNode = dynamic_cast<sim_mob::MultiNode*>(nd);
			if (multNode) {
				multNode->roadSegmentsAt.insert(currSec->generatedSegment);
			}
		}

		//Retrieve the generated segment
		sim_mob::RoadSegment* rs = currSec->generatedSegment;
		rs->originalDB_ID.setProps("aimsun-id", currSec->id);

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
				if (nextSection) {
					throw std::runtime_error("UniNode has competing outgoing Sections.");
				}
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
	  const unsigned long  aaa = a->getRoadSegment()->getSegmentID();
	  const unsigned int  aaaa = a->getLaneID() ;

	  const sim_mob::Lane* b = (d->getLaneFrom());
	  const unsigned int  bb = b->getRoadSegment()->getLink()->getLinkId();
	  const unsigned long  bbb = b->getRoadSegment()->getSegmentID();
	  const unsigned int  bbbb = b->getLaneID() ;
	  ///////////////////////////////////////////////////////
	  const sim_mob::Lane* a1 = (c->getLaneTo());
	  const unsigned int  aa1 = a1->getRoadSegment()->getLink()->getLinkId();
	  const unsigned long  aaa1 = a1->getRoadSegment()->getSegmentID();
	  const unsigned int  aaaa1 = a1->getLaneID() ;

	  const sim_mob::Lane* b1 = (d->getLaneTo());
	  const unsigned int  bb1 = b1->getRoadSegment()->getLink()->getLinkId();
	  const unsigned long  bbb1 = b1->getRoadSegment()->getSegmentID();
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
		throw std::runtime_error("Turning doesn't match with Sections and Nodes.");
	}

	//Skip Turnings which meet at UniNodes; these will be handled elsewhere.
	sim_mob::Node* meetingNode = src.fromSection->toNode->generatedNode;
	if (dynamic_cast<UniNode*>(meetingNode)) {
		return;
	}

	//Essentially, just expand each turning into a set of LaneConnectors.
	//TODO: This becomes slightly more complex at RoadSegmentNodes, since these
	//      only feature one primary connector per Segment pair.
	for (int fromLaneID=src.fromLane.first; fromLaneID<=src.fromLane.second; fromLaneID++) {
		for (int toLaneID=src.toLane.first; toLaneID<=src.toLane.second; toLaneID++) {
			//Bounds check: temp
			/*if (fromLaneID>=src.fromSection->generatedSegment->lanes.size() ||
				toLaneID >= src.toSection->generatedSegment->lanes.size()) {
				std::cout <<"SKIPPING LANE\n";
				continue;
			}*/

			//Process
			sim_mob::LaneConnector* lc = new sim_mob::LaneConnector();
			int adjustedFromLaneId  = src.fromSection->generatedSegment->getAdjustedLaneId(fromLaneID);
			int adjustedToLaneId  = src.toSection->generatedSegment->getAdjustedLaneId(toLaneID);
			lc->laneFrom = src.fromSection->generatedSegment->lanes.at(adjustedFromLaneId);
			lc->laneTo = src.toSection->generatedSegment->lanes.at(adjustedToLaneId);
//			lc->laneFrom = src.fromSection->generatedSegment->lanes.at(fromLaneID);
//			lc->laneTo = src.toSection->generatedSegment->lanes.at(toLaneID);

			//just a check to avoid connecting pedestrian and non pedestrian lanes
			int i = 0;
			if(lc->laneFrom->is_pedestrian_lane()) i++;
			if(lc->laneTo->is_pedestrian_lane()) i++;

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

void sim_mob::aimsun::Loader::LoadERPData(const std::string& connectionStr,
		std::map<std::string,std::vector<sim_mob::ERP_Surcharge*> > &erp_surcharge_pool,
		std::map<std::string,sim_mob::ERP_Gantry_Zone*>& erp_gantry_zone_pool,
		std::map<std::string,sim_mob::ERP_Section*>& erp_section_pool)
{
	DatabaseLoader loader(connectionStr);
	loader.LoadERP_Surcharge(erp_surcharge_pool);
	loader.LoadERP_Section(erp_section_pool);
	loader.LoadERP_Gantry_Zone(erp_gantry_zone_pool);
}
bool sim_mob::aimsun::Loader::createTable(const std::string& connectionStr,
		std::string& table_name)
{
	DatabaseLoader loader(connectionStr);
	bool res = loader.CreateTable(table_name);
	return res;
}
bool sim_mob::aimsun::Loader::insertData2TravelTimeTmpTable(const std::string& connectionStr,
		std::string& table_name,
		sim_mob::Link_travel_time& data)
{
	DatabaseLoader loader(connectionStr);
	bool res = loader.InsertData2TravelTimeTmpTable(table_name,data);
	return res;
}
bool sim_mob::aimsun::Loader::insertCSV2Table(const std::string& connectionStr,
		std::string& table_name,
		std::string& csvFileName)
{
	DatabaseLoader loader(connectionStr);
	bool res = loader.InsertCSV2Table(table_name,csvFileName);
	return res;
}
bool sim_mob::aimsun::Loader::truncateTable(const std::string& connectionStr,
			std::string& table_name)
{
	DatabaseLoader loader(connectionStr);
	bool res=loader.TruncateTable(table_name);
	return res;
}
bool sim_mob::aimsun::Loader::excuString(const std::string& connectionStr,
					std::string& str)
{
	DatabaseLoader loader(connectionStr);
	bool res=loader.ExcuString(str);
	return res;
}
void sim_mob::aimsun::Loader::LoadDefaultTravelTimeData(const std::string& connectionStr,
		std::map<std::string,std::vector<sim_mob::Link_travel_time*> >& link_default_travel_time_pool)
{
	DatabaseLoader loader(connectionStr);
	loader.Loadlink_default_travel_time(link_default_travel_time_pool);
}
bool sim_mob::aimsun::Loader::LoadRealTimeTravelTimeData(const std::string& connectionStr,
		std::string &table_name,
		std::map<std::string,std::vector<sim_mob::Link_travel_time*> >& link_realtime_travel_time_pool)
{
	DatabaseLoader loader(connectionStr);
	bool res = loader.Loadlink_realtime_travel_time(table_name,link_realtime_travel_time_pool);
	return res;
}
//void sim_mob::aimsun::Loader::initLoader(const std::string& connectionStr)
//{
//	std::cout << "initLoader" << std::endl;
//	mysocisql_ = soci::session(soci::postgresql, connectionStr);
//}
void sim_mob::aimsun::Loader::LoadPathSetData(const std::string& connectionStr,
		std::map<std::string,sim_mob::SinglePath*>& pathPool,
		std::map<std::string,SinglePath*> &waypoint_singlepathPool,
		std::map<std::string,sim_mob::PathSet* >& pathSetPool)
{
//	std::cout << "Attempting to connect to database (pathset)" << std::endl;
//	if(!myloader)
//		myloader = new DatabaseLoader2(connectionStr);
//	//Connection string will look something like this:
//	//"host=localhost port=5432 dbname=SimMobility_DB user=postgres password=XXXXX"
////	DatabaseLoader loader(connectionStr);
//	myloader->LoadSinglePathDB(pathPool,waypoint_singlepathPool);
////	std::cout<<"LoadSinglePathDB: "<<waypoint_singlepathPool.size()<<std::endl;
//	myloader->LoadPathSetDB(pathSetPool);
////	loader.LoadPathPoolDBDB(PathPoolDBPool);
//	std::cout << ">load pathset Success." << std::endl;
}
bool sim_mob::aimsun::Loader::LoadSinglePathDBwithId2(const std::string& connectionStr,
			std::map<std::string,sim_mob::SinglePath*>& waypoint_singlepathPool,
			std::string& pathset_id,
			std::vector<sim_mob::SinglePath*>& spPool)
{
	DatabaseLoader loader(connectionStr);
	bool res = loader.LoadSinglePathDBwithId2(waypoint_singlepathPool,pathset_id,spPool);
	return res;
}
bool sim_mob::aimsun::Loader::LoadPathSetDBwithId(const std::string& connectionStr,
		std::map<std::string,sim_mob::PathSet* >& pool,
		std::string& pathset_id)
{
	DatabaseLoader loader(connectionStr);
	bool res = loader.LoadPathSetDBwithId(pool,pathset_id);
	return res;
}
bool sim_mob::aimsun::Loader::LoadOnePathSetDBwithId(const std::string& connectionStr,
		sim_mob::PathSet& ps,
				std::string& pathset_id)
{
	DatabaseLoader loader(connectionStr);
	bool res = loader.LoadOnePathSetDBwithId(pathset_id,ps);
	return res;
}
bool sim_mob::aimsun::Loader::LoadPathSetDataWithId(const std::string& connectionStr,
		std::map<std::string,sim_mob::SinglePath*>& pathPool,
		std::map<std::string,SinglePath*> &waypoint_singlepathPool,
		std::map<std::string,sim_mob::PathSet* >& pathSetPool,std::string& pathset_id)
{
//	bool res=false;
//	std::cout << "LoadPathSetDataWithId: loading " <<pathset_id<<" data from db"<< std::endl;
//	//Connection string will look something like this:
//	//"host=localhost port=5432 dbname=SimMobility_DB user=postgres password=XXXXX"
//	if(!myloader)
//			myloader = new DatabaseLoader2(connectionStr);
////	DatabaseLoader loader(connectionStr);
//	myloader->LoadSinglePathDBwithId(pathPool,waypoint_singlepathPool,pathset_id);
////	std::cout<<"LoadSinglePathDB: "<<waypoint_singlepathPool.size()<<std::endl;
//	res = myloader->LoadPathSetDBwithId(pathSetPool,pathset_id);
////	loader.LoadPathPoolDBDB(PathPoolDBPool);
////	std::cout << ">load pathset Success." << std::endl;
//
//	return res;
}
void sim_mob::aimsun::Loader::SavePathSetData(const std::string& connectionStr,
		std::map<std::string,sim_mob::SinglePath*>& pathPool,
		std::map<std::string,sim_mob::PathSet* >& pathSetPool)
{
	std::cout << "Attempting to connect to database (pathset)" << std::endl;
	//Connection string will look something like this:
	//"host=localhost port=5432 dbname=SimMobility_DB user=postgres password=XXXXX"
	DatabaseLoader loader(connectionStr);
//	for(std::map<std::string,sim_mob::SinglePath*>::iterator it=pathPool.begin();it!=pathPool.end();++it)
//	{
//		sim_mob::SinglePath* sp = (*it).second;
//		sim_mob::SinglePathDB *data = sp->dbData;
//	}
	loader.InsertSinglePath2DB(pathPool);
	loader.InsertPathSet2DB(pathSetPool);
}
void sim_mob::aimsun::Loader::SaveOneSinglePathData(const std::string& connectionStr,
		std::map<std::string,sim_mob::SinglePath*>& pathPool)
{
	DatabaseLoader loader(connectionStr);
	loader.InsertSinglePath2DB(pathPool);
}
void sim_mob::aimsun::Loader::SaveOnePathSetData(const std::string& connectionStr,
		std::map<std::string,sim_mob::PathSet* >& pathSetPool)
{
	DatabaseLoader loader(connectionStr);
	loader.InsertPathSet2DB(pathSetPool);
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

	//Step 1.1: Load "new style" objects, which don't require any post-processing.
	loader.LoadBusSchedule(getStoredProcedure(storedProcs, "bus_schedule", false), config.getBusSchedule());

	//TODO: Possibly re-enable later.
	//if (prof) { prof->logGenericEnd("Database", "main-prof"); }

	//TODO: Possibly re-enable later.
	//if (prof) { prof->logGenericStart("PostProc", "main-prof"); }

	//Step Two: Translate
	loader.DecorateAndTranslateObjects();

	//Step Three: Perform data-guided cleanup.
	loader.PostProcessNetwork();

	//Step Four: Save
	loader.SaveSimMobilityNetwork(rn, tcs);

	//Temporary Workaround: Links need to know about any "reverse" direction Links.
	//The StreetDirectory can provide this, but that hasn't been initialized yet.
	//So, we set a temporary flag in the Link class itself.
	{
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
	}

	//Temporary workaround; Cut lanes short/extend them as reuquired.
	for (map<int,Section>::const_iterator it=loader.sections().begin(); it!=loader.sections().end(); it++) {
		TMP_TrimAllLaneLines(it->second.generatedSegment, it->second.HACK_LaneLinesStartLineCut, true);
		TMP_TrimAllLaneLines(it->second.generatedSegment, it->second.HACK_LaneLinesEndLineCut, false);
	}
	for(vector<sim_mob::Link*>::iterator it = rn.links.begin(); it!= rn.links.end();it++)
		(*it)->extendPolylinesBetweenRoadSegments();

	//TODO: Possibly re-enable later.
	//if (prof) { prof->logGenericEnd("PostProc", "main-prof"); }

	//added by Melani - to compute lane zero lengths of road segments
	for (map<int,Section>::const_iterator it=loader.sections().begin(); it!=loader.sections().end(); it++) {
		it->second.generatedSegment->laneZeroLength = it->second.generatedSegment->computeLaneZeroLength();
	}
	//add by xuyan, load in boundary segments
	//Step Four: find boundary segment in road network using start-node(x,y) and end-node(x,y)
#ifndef SIMMOB_DISABLE_MPI
	if (ConfigManager::GetInstance().FullConfig().using_MPI)
	{
		loader.TransferBoundaryRoadSegment();
	}
#endif

	std::cout <<"AIMSUN Network successfully imported.\n";

	loader.LoadPTBusDispatchFreq(getStoredProcedure(storedProcs, "pt_bus_dispatch_freq", false), config.getPT_bus_dispatch_freq());
	loader.LoadPTBusRoutes(getStoredProcedure(storedProcs, "pt_bus_routes", false), config.getPT_bus_routes(), config.getRoadSegments_Map());
	loader.LoadPTBusStops(getStoredProcedure(storedProcs, "pt_bus_stops", false), config.getPT_bus_stops(), config.getBusStops_Map());
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
	sim_mob::Conflux* conflux = nullptr;

	//Make a temporary map of road nodes-to-road segments
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
		conflux = new sim_mob::Conflux(*i, mtxStrat);

		std::map<const sim_mob::MultiNode*, std::set<const sim_mob::RoadSegment*> >::iterator segsAt = roadSegmentsAt.find(*i);
		if (segsAt!=roadSegmentsAt.end()) {
			for (std::set<const sim_mob::RoadSegment*>::iterator segmt=segsAt->second.begin(); segmt!=segsAt->second.end(); segmt++) {
				sim_mob::Link* lnk = (*segmt)->getLink();
				std::vector<sim_mob::RoadSegment*> upSegs;
				std::vector<sim_mob::RoadSegment*> downSegs;

				//If the Link in question *ends* at the Node we are considering for a Conflux.
				if(lnk->getEnd() == (*i))
				{
					//NOTE: There will *only* be upstream segments in this case.
					upSegs = lnk->getSegments();
					conflux->upstreamSegmentsMap.insert(std::make_pair(lnk, upSegs));
					conflux->virtualQueuesMap.insert(std::make_pair(lnk, std::deque<sim_mob::Person*>()));
				}
				else if (lnk->getStart() == (*i))
				{
					//NOTE: There will *only* be downstream segments in this case.
					downSegs = lnk->getSegments();
					conflux->downstreamSegments.insert(downSegs.begin(), downSegs.end());
				}

				// set conflux pointer to the segments and create SegmentStats for the segment
				for(std::vector<sim_mob::RoadSegment*>::iterator segIt = upSegs.begin();
						segIt != upSegs.end(); segIt++)
				{
					sim_mob::RoadSegment* rdSeg = *segIt;
					if(rdSeg->parentConflux == nullptr)
					{
						// assign only if not already assigned
						rdSeg->parentConflux = conflux;
						conflux->segmentAgents.insert(std::make_pair(rdSeg, new SegmentStats(rdSeg, rdSeg->getLaneZeroLength())));
						multinode_confluxes.insert(std::make_pair(segsAt->first, conflux));
					}
					else if(rdSeg->parentConflux != conflux)
					{
						debugMsgs << "\nProcessConfluxes\tparentConflux is being re-assigned for segment " << rdSeg->getStartEnd()<< std::endl;
						throw std::runtime_error(debugMsgs.str());
					}
				}
			} // for
		}
		conflux->resetOutputBounds();
		confluxes.insert(conflux);
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


