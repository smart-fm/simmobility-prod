//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/**
 * Pt_NetworkSqlDao.cpp
 *
 *  Created on: Feb 25th, 2015
 *  \author: Prabhuraj
 */

#include "PT_NetworkSqlDao.hpp"

#include "PT_DatabaseHelper.hpp"

#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"

using namespace sim_mob;
using namespace sim_mob::db;



PT_VerticesSqlDao::PT_VerticesSqlDao(DB_Connection& connection,std::string query):
		SqlAbstractDao<PT_NetworkVertices>(connection, "", "", "", "",
		query, "")
{
	/*
	ConfigParams& cfg = ConfigManager::GetInstanceRW().FullConfig();
	const std::string DB_STORED_PROC_PT_VERTICES = cfg.getDatabaseProcMappings().procedureMappings["pt_vertices"];
	const std::string DB_GETALL_PT_VERTICES = "SELECT * FROM " + DB_STORED_PROC_PT_VERTICES;
	*/
}

PT_VerticesSqlDao::~PT_VerticesSqlDao()
{}

void PT_VerticesSqlDao::fromRow(Row& result, PT_NetworkVertices& outObj) {
	outObj.setStopId(result.get<std::string>(DB_FIELD_PT_VERTICES_STOP_ID,EMPTY_STRING));
	outObj.setStopCode(result.get<std::string>(DB_FIELD_PT_VERTICES_STOP_CODE,EMPTY_STRING));
	outObj.setStopName(result.get<std::string>(DB_FIELD_PT_VERTICES_STOP_NAME,EMPTY_STRING));
	outObj.setStopLatitude(result.get<double>(DB_FIELD_PT_VERTICES_STOP_LATITUDE));
	outObj.setStopLongitude(result.get<double>(DB_FIELD_PT_VERTICES_STOP_LONGITUDE));
	outObj.setEzlinkName(result.get<std::string>(DB_FIELD_PT_VERTICES_EZLINK_NAME,EMPTY_STRING));
	outObj.setStopType(result.get<int>(DB_FIELD_PT_VERTICES_STOP_TYPE));
	outObj.setStopDesc(result.get<std::string>(DB_FIELD_PT_VERTICES_STOP_DESCRIPTION,EMPTY_STRING));
}

void PT_VerticesSqlDao::toRow(PT_NetworkVertices& data, Parameters& outParams, bool update) {
}

Pt_EdgesSqlDao::Pt_EdgesSqlDao(DB_Connection& connection,std::string query):
		SqlAbstractDao<PT_NetworkEdges>(connection, "", "", "", "",
				query, "")
{
	/*
	ConfigParams& cfg = ConfigManager::GetInstanceRW().FullConfig();
	const std::string DB_STORED_PROC_PT_EDGES = cfg.getDatabaseProcMappings().procedureMappings["pt_edges"];
	const std::string DB_GETALL_PT_VERTICES = "SELECT * FROM " + DB_STORED_PROC_PT_EDGES;
	SqlAbstractDao<PT_NetworkEdges>(connection, "", "", "", "",DB_GETALL_PT_VERTICES, "");
	*/
}

Pt_EdgesSqlDao::~Pt_EdgesSqlDao()
{}

void Pt_EdgesSqlDao::fromRow(Row& result, PT_NetworkEdges& outObj) {
	outObj.setStartStop(result.get<std::string>(DB_FIELD_PT_EDGES_START_STOP,EMPTY_STRING));
	outObj.setEndStop(result.get<std::string>(DB_FIELD_PT_EDGES_END_STOP,EMPTY_STRING));
	outObj.setR_Type(result.get<std::string>(DB_FIELD_PT_EDGES_R_TYPE,EMPTY_STRING));
	outObj.setRoadIndex(result.get<std::string>(DB_FIELD_PT_EDGES_ROAD_INDEX,EMPTY_STRING));
	outObj.setRoadEdgeId(result.get<std::string>(DB_FIELD_PT_EDGES_ROAD_EDGE_ID,EMPTY_STRING));
	outObj.setServiceLines(result.get<std::string>(DB_FIELD_PT_EDGES_R_SERVICE_LINES,EMPTY_STRING));
	outObj.setLinkTravelTime(result.get<double>(DB_FIELD_PT_EDGES_LINK_TRAVEL_TIME));
	outObj.setEdgeId(result.get<int>(DB_FIELD_PT_EDGES_EDGE_ID));
	outObj.setWaitTime(result.get<double>(DB_FIELD_PT_EDGES_WAIT_TIME));
	outObj.setWalkTime(result.get<double>(DB_FIELD_PT_EDGES_WALK_TIME));
	outObj.setTransitTime(result.get<double>(DB_FIELD_PT_EDGES_TRANSIT_TIME));
	outObj.setTransferPenalty(result.get<double>(DB_FIELD_PT_EDGES_TRANSFER_PENALTY));
	outObj.setTransitTime(result.get<double>(DB_FIELD_PT_EDGES_DAY_TRANSIT_TIME));
	outObj.setDist(result.get<double>(DB_FIELD_PT_EDGES_DIST));

}

void Pt_EdgesSqlDao::toRow(PT_NetworkEdges& data, Parameters& outParams, bool update) {
}

