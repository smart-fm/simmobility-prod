//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "PT_NetworkEntities.hpp"

#include <sstream>
#include "conf/ConfigManager.hpp"
#include "conf/Constructs.hpp"
#include "conf/ConfigParams.hpp"
#include "conf/RawConfigParams.hpp"
#include "database/DB_Connection.hpp"
#include "database/pt_network_dao/PT_NetworkSqlDao.hpp"
#include "geospatial/streetdir/StreetDirectory.hpp"
#include "geospatial/streetdir/RailTransit.hpp"
#include "util/LangHelpers.hpp"
#include "util/Utils.hpp"



using namespace std;
using namespace sim_mob;
PT_Network sim_mob::PT_NetworkCreater::instance;
PT_Network sim_mob::PT_NetworkCreater::instance2;

void PT_NetworkCreater::init()
{
	const std::string DB_STORED_PROC_PT_EDGES = ConfigManager::GetInstanceRW().FullConfig().getDatabaseProcMappings().procedureMappings["pt_edges"];
	const std::string DB_STORED_PROC_PT_VERTICES = ConfigManager::GetInstanceRW().FullConfig().getDatabaseProcMappings().procedureMappings["pt_vertices"];
	instance.init(DB_STORED_PROC_PT_VERTICES, DB_STORED_PROC_PT_EDGES);
	instance2.init(DB_STORED_PROC_PT_VERTICES, DB_STORED_PROC_PT_EDGES);
}
namespace
{
void loadMRTData(soci::session& sql_, std::map<std::string, TrainStop*>& mrtStopsMap)
{
	//Reading the MRT data
	std::string storedProc = sim_mob::ConfigManager::GetInstance().FullConfig().getDatabaseProcMappings().procedureMappings["mrt_road_segments"];
	std::stringstream query;
	query << "select * from " << storedProc;
	soci::rowset<soci::row> rs = (sql_.prepare << query.str());
	for (soci::rowset<soci::row>::const_iterator it = rs.begin(); it != rs.end(); ++it)
	{
	   soci::row const& row = *it;
	   std::string mrtstopid = row.get<std::string>(0);
	   int roadsegmentId = row.get<unsigned int>(1);
	   //MRT stop id can be a slash '/' separated list of IDs in case of interchanges. We need to split it into individual stop ids
	   if(mrtstopid.find('/') == std::string::npos)
	   {
		   if(mrtStopsMap.find(mrtstopid) == mrtStopsMap.end())
		   {
			   TrainStop* mrtStopObj = new TrainStop(mrtstopid);
			   mrtStopsMap[mrtstopid] = mrtStopObj;
		   }
		   mrtStopsMap[mrtstopid]->addAccessRoadSegment(roadsegmentId);
	   }
	   else
	   {
		   TrainStop* mrtStopObj = nullptr;
		   std::stringstream ss(mrtstopid);
		   std::string singleMrtStopId;
		   while (std::getline(ss, singleMrtStopId, '/'))
		   {
			   if(!mrtStopObj)
			   {
				   if(mrtStopsMap.find(singleMrtStopId) == mrtStopsMap.end())
				   {
					   mrtStopObj = new TrainStop(mrtstopid); //note: the original '/' separated string of ids must be passed to constructor; check constructor implementation for details.
					   mrtStopsMap[singleMrtStopId] = mrtStopObj;
				   }
				   else
				   {
					   mrtStopObj = mrtStopsMap[singleMrtStopId];
				   }
			   }
			   else if(mrtStopsMap.find(singleMrtStopId) == mrtStopsMap.end())
			   {
				   mrtStopsMap[singleMrtStopId] = mrtStopObj;
			   }
		   }
		   mrtStopObj->addAccessRoadSegment(roadsegmentId);
	   }
	}
}

void loadRailTransitGraphData(soci::session& sql_, std::set<string>& rtVertices, std::vector<RTS_NetworkEdge>& rtEdges)
{
	//Reading the MRT data
	std::string storedProc = sim_mob::ConfigManager::GetInstance().FullConfig().getDatabaseProcMappings().procedureMappings["rail_transit_edges"];
	if(storedProc.empty()) { return; }
	std::stringstream query;
	query << "select * from " << storedProc;
	soci::rowset<soci::row> rs = (sql_.prepare << query.str());
	for (soci::rowset<soci::row>::const_iterator it = rs.begin(); it != rs.end(); ++it)
	{
	   soci::row const& row = *it;
	   RTS_NetworkEdge rtNwEdge;
	   rtNwEdge.setFromStationId(row.get<std::string>(0));
	   rtNwEdge.setToStationId(row.get<std::string>(1));
	   rtNwEdge.setEdgeTravelTime(row.get<double>(2));
	   rtNwEdge.setTransferEdge((row.get<std::string>(3) == "TRF"));

	   rtVertices.insert(rtNwEdge.getFromStationId()); //set container eliminates duplicates
	   rtVertices.insert(rtNwEdge.getToStationId()); //set container eliminates duplicates
	   rtEdges.push_back(rtNwEdge);
	}
}

void printTransferPoints(std::string o, std::string d)
{
	const RailTransit& railTransit = RailTransit::getInstance();
	std::vector<string> boardAlightSeq;
	boardAlightSeq = railTransit.fetchBoardAlightStopSeq(o,d);
	cout << o << "," << d << ": ";
	for(auto& stn : boardAlightSeq)
	{
		cout << stn << ",";
	}
	cout << endl;
}
}

void PT_Network::init(const std::string& storedProcForVertex, const std::string& storeProceForEdges)
{
	vector<PT_NetworkVertex> PublicTransitVertices;
	vector<PT_NetworkEdge> PublicTransitEdges;
	const std::string& dbId = ConfigManager::GetInstance().FullConfig().networkDatabase.database;
	Database database = ConfigManager::GetInstance().FullConfig().constructs.databases.at(dbId);
	std::string cred_id = ConfigManager::GetInstance().FullConfig().networkDatabase.credentials;

	Credential credentials = ConfigManager::GetInstance().FullConfig().constructs.credentials.at(cred_id);
	std::string username = credentials.getUsername();
	std::string password = credentials.getPassword(false);
	sim_mob::db::DB_Config dbConfig(database.host, database.port, database.dbName, username, password);

	const std::string DB_GETALL_PT_EDGES_QUERY = "SELECT * FROM " + storeProceForEdges;
	const std::string DB_GETALL_PT_VERTICES_QUERY = "SELECT * FROM " + storedProcForVertex;

	// Connect to database and load data.
	sim_mob::db::DB_Connection conn(sim_mob::db::POSTGRES, dbConfig);
	conn.connect();
	if (conn.isConnected()) {
		PT_VerticesSqlDao pt_verticesDao(conn,DB_GETALL_PT_VERTICES_QUERY);
		pt_verticesDao.getAll(PublicTransitVertices);

		Pt_EdgesSqlDao pt_edgesDao(conn,DB_GETALL_PT_EDGES_QUERY);
		pt_edgesDao.getAll(PublicTransitEdges);
	}

	// Building a Public transit map for edges and vertices
	for(vector<PT_NetworkVertex>::const_iterator ptVertexIt=PublicTransitVertices.begin();ptVertexIt!=PublicTransitVertices.end();ptVertexIt++)
	{
		PT_NetworkVertexMap[(*ptVertexIt).getRoadItemId()]=*ptVertexIt;
	}
	for(vector<PT_NetworkEdge>::const_iterator ptEdgeIt=PublicTransitEdges.begin();ptEdgeIt!=PublicTransitEdges.end();ptEdgeIt++)
	{
		PT_NetworkEdgeMap[ptEdgeIt->getEdgeId()]=*ptEdgeIt;
	}

	//Read MRT data
	soci::session& sql_ = conn.getSession<soci::session>();

	std::string storedProc = sim_mob::ConfigManager::GetInstance().FullConfig().getDatabaseProcMappings().procedureMappings["mrt_road_segments"];
	std::stringstream query;
	query << "select * from " << storedProc;
	soci::rowset<soci::row> rs = (sql_.prepare << query.str());
	for (soci::rowset<soci::row>::const_iterator it = rs.begin(); it != rs.end(); ++it)
	{
	   soci::row const& row = *it;
	   std::string mrtstopid = row.get<std::string>(0);
	   int roadsegmentId = row.get<unsigned int>(1);
	   //MRT stop id can be a slash '/' separated list of IDs in case of interchanges. We need to split it into individual stop ids
	   if(mrtstopid.find('/') == std::string::npos)
	   {
		   if(MRTStopsMap.find(mrtstopid) == MRTStopsMap.end())
		   {
			   TrainStop* mrtStopObj = new TrainStop(mrtstopid);
			   MRTStopsMap[mrtstopid] = mrtStopObj;
		   }
		   MRTStopsMap[mrtstopid]->addAccessRoadSegment(roadsegmentId);
	   }
	   else
	   {
		   TrainStop* mrtStopObj = new TrainStop(mrtstopid);
		   MRTStopsMap[mrtstopid] = mrtStopObj;
		   std::stringstream ss(mrtstopid);
		   std::string singleMrtStopId;
		   while (std::getline(ss, singleMrtStopId, '/'))
		   {
			   if(!mrtStopObj)
			   {
				   if(MRTStopsMap.find(singleMrtStopId) == MRTStopsMap.end())
				   {
					   mrtStopObj = new TrainStop(mrtstopid); //note: the original '/' separated string of ids must be passed to constructor; check constructor implementation for details.
					   MRTStopsMap[singleMrtStopId] = mrtStopObj;
				   }
				   else
				   {
					   mrtStopObj = MRTStopsMap[singleMrtStopId];
				   }
			   }
			   else if(MRTStopsMap.find(singleMrtStopId) == MRTStopsMap.end())
			   {
				   MRTStopsMap[singleMrtStopId] = mrtStopObj;
			   }
		   }
		   mrtStopObj->addAccessRoadSegment(roadsegmentId);
	   }
	}

	loadMRTData(sql_, MRTStopsMap);

	std::set<string> mrtStationIds;
	std::vector<RTS_NetworkEdge> mrtEdges;
	loadRailTransitGraphData(sql_, mrtStationIds, mrtEdges);
	RailTransit::getInstance().initGraph(mrtStationIds, mrtEdges);


	cout << "Public Transport network loaded\n";
}

sim_mob::PT_Network::~PT_Network()
{
	while(!MRTStopsMap.empty())
	{
		std::map<std::string, TrainStop*>::iterator trainStopIt = MRTStopsMap.begin();
		TrainStop* stop = trainStopIt->second;
		std::vector<std::string> otherIdsForSameTrainStop = stop->getTrainStopIds();
		delete stop;
		MRTStopsMap.erase(trainStopIt);
		for(std::string& id : otherIdsForSameTrainStop) //there may be multiple stop_id's pointing to the same stop object (for interchanges)
		{
			trainStopIt = MRTStopsMap.find(id);
			if(trainStopIt!=MRTStopsMap.end())
			{
				MRTStopsMap.erase(trainStopIt);
			}
		}
	}
}

int PT_Network::getVertexTypeFromStopId(std::string stopId)
{
	if(PT_NetworkVertexMap.find(stopId) != PT_NetworkVertexMap.end())
	{
		return PT_NetworkVertexMap.find(stopId)->second.getStopType();
	}
	return -1;
}

TrainStop* PT_Network::findMRT_Stop(const std::string& stopId) const
{
	//stop id can be a slash '/' separated list of MRT stop ids.
	//all MRT stops in the '/' separated list point to the same TrainStop object in MRTStopsMap
	std::stringstream ss(stopId);
	std::string singleMrtStopId;
	std::getline(ss, singleMrtStopId, '/');
	std::map<std::string, TrainStop*>::const_iterator trainStopIt = MRTStopsMap.find(singleMrtStopId);
	if (trainStopIt != MRTStopsMap.end())
	{
		return trainStopIt->second;
	}
	else
	{
		return nullptr;
	}
}

PT_NetworkEdge::PT_NetworkEdge():startStop(""),endStop(""),rType(""),road_index(""),roadEdgeId(""),
rServiceLines(""),linkTravelTimeSecs(0),edgeId(0),waitTimeSecs(0),walkTimeSecs(0),transitTimeSecs(0),
transferPenaltySecs(0),dayTransitTimeSecs(0),distKMs(0), edgeType(sim_mob::UNKNOWN_EDGE)
{}

PT_NetworkEdge::~PT_NetworkEdge() {}

PT_NetworkVertex::PT_NetworkVertex():stopId(""),stopCode(""),stopName(""),stopLatitude(0),
		stopLongitude(0),ezlinkName(""),stopType(0),stopDesc("")
{}

PT_NetworkVertex::~PT_NetworkVertex()
{
}
