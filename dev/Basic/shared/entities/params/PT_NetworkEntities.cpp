//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "PT_NetworkEntities.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/Constructs.hpp"
#include "conf/ConfigParams.hpp"
#include "conf/RawConfigParams.hpp"
#include "database/DB_Connection.hpp"
#include "database/pt_network_dao/PT_NetworkSqlDao.hpp"
#include "geospatial/streetdir/StreetDirectory.hpp"
#include "util/Utils.hpp"


using namespace std;
using namespace sim_mob;
PT_Network sim_mob::PT_Network::instance_;

void PT_Network::init()
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


	const std::string DB_STORED_PROC_PT_EDGES = ConfigManager::GetInstanceRW().FullConfig().getDatabaseProcMappings().procedureMappings["pt_edges"];
	const std::string DB_GETALL_PT_EDGES_QUERY = "SELECT * FROM " + DB_STORED_PROC_PT_EDGES;
	const std::string DB_STORED_PROC_PT_VERTICES = ConfigManager::GetInstanceRW().FullConfig().getDatabaseProcMappings().procedureMappings["pt_vertices"];
	const std::string DB_GETALL_PT_VERTICES_QUERY = "SELECT * FROM " + DB_STORED_PROC_PT_VERTICES;

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

	//Reading the MRT data
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
	   if(MRTStopsMap.find(mrtstopid) == MRTStopsMap.end())
	   {
		   TrainStop mrtStopObj(mrtstopid, roadsegmentId);
		   MRTStopsMap[mrtstopid]=mrtStopObj;
	   }
	   else{
		   MRTStopsMap[mrtstopid].addAccessRoadSegment(roadsegmentId);
	   }
	}
	cout << "Public Transport network loaded\n";
}
int PT_Network::getVertexTypeFromStopId(std::string stopId)
{
	if(PT_NetworkVertexMap.find(stopId) != PT_NetworkVertexMap.end())
	{
		return PT_NetworkVertexMap.find(stopId)->second.getStopType();
	}
	return -1;
}

TrainStop* PT_Network::findMRT_Stop(const std::string& stopId){
	if(MRTStopsMap.find(stopId) != MRTStopsMap.end()){
		return &MRTStopsMap[stopId];
	}
	else {
		return nullptr;
	}
}

PT_NetworkEdge::PT_NetworkEdge():startStop(""),endStop(""),rType(""),road_index(""),roadEdgeId(""),
rServiceLines(""),linkTravelTimeSecs(0),edgeId(0),waitTimeSecs(0),walkTimeSecs(0),transitTimeSecs(0),
transferPenaltySecs(0),dayTransitTimeSecs(0),distKMs(0)
{}

PT_NetworkEdge::~PT_NetworkEdge() {}

PT_NetworkVertex::PT_NetworkVertex():stopId(""),stopCode(""),stopName(""),stopLatitude(0),
		stopLongitude(0),ezlinkName(""),stopType(0),stopDesc("")
{}

PT_NetworkVertex::~PT_NetworkVertex()
{
}
