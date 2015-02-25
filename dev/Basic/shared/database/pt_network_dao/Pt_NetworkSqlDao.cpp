//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * Pt_NetworkSqlDao.cpp
 *
 *  Created on: Feb 25th, 2015
 *      Author: Prabhuraj
 */

#include "Pt_NetworkSqlDao.hpp"

#include <boost/lexical_cast.hpp>
#include "DatabaseHelper.hpp"

using namespace sim_mob;
using namespace sim_mob::db;
using namespace sim_mob::medium;

Pt_VerticesSqlDao::Pt_VerticesSqlDao(DB_Connection& connection)
: SqlAbstractDao<Pt_network_vertices*>(connection, "", "", "", "", DB_STORED_PROC_PT_VERTICES, "")
{}

Pt_VerticesSqlDao::~Pt_VerticesSqlDao()
{}

void Pt_VerticesSqlDao::fromRow(Row& result, Pt_network_vertices& outObj) {
	outObj.setStopId(boost::lexical_cast<std::string>(result.get<std::String>(DB_FIELD_PT_VERTICES_STOP_ID)));
	outObj.setStopCode(result.get<std::String>(DB_FIELD_PT_VERTICES_STOP_CODE));
	outObj.setStopName(result.get<std::String>(DB_FIELD_PT_VERTICES_STOP_NAME));
	outObj.setStopLatitude(result.get<double>(DB_FIELD_PT_VERTICES_STOP_LATITUDE));
	outObj.setStopLongitude(result.get<double>(DB_FIELD_PT_VERTICES_STOP_LONGITUDE));
	outObj.setEzlinkName(result.get<std::String>(DB_FIELD_PT_VERTICES_EZLINK_NAME));
	outObj.setStopType(result.get<int>(DB_FIELD_PT_VERTICES_STOP_TYPE));
	outObj.setStopDesc(result.get<std::String>(DB_FIELD_PT_VERTICES_STOP_DESCRIPTION));
}

void Pt_VerticesSqlDao::toRow(pt_network_edges& data, Parameters& outParams, bool update) {
}

Pt_EdgesSqlDao::Pt_EdgesSqlDao(DB_Connection& connection)
: SqlAbstractDao<pt_network_edges*>(connection, "", "", "", "", DB_STORED_PROC_PT_EDGES, "")
{}

Pt_EdgesSqlDao::~Pt_EdgesSqlDao()
{}

void Pt_EdgesSqlDao::fromRow(Row& result, pt_network_edges& outObj) {
	outObj.setStartStop(boost::lexical_cast<std::string>(result.get<std::String>(DB_FIELD_PT_EDGES_START_STOP)));
	outObj.setEndStop(result.get<std::String>(DB_FIELD_PT_EDGES_END_STOP));
	outObj.setR_Type(result.get<std::String>(DB_FIELD_PT_EDGES_R_TYPE));
	outObj.setRoadIndex(result.get<int>(DB_FIELD_PT_EDGES_ROAD_INDEX));
	outObj.setRoadEdgeId(result.get<std::String>(DB_FIELD_PT_EDGES_ROAD_EDGE_ID));
	outObj.setServiceLines()(result.get<std::String>(DB_FIELD_PT_EDGES_R_SERVICE_LINES));
	outObj.setLinkTravelTime(result.get<double>(DB_FIELD_PT_EDGES_LINK_TRAVEL_TIME));
	outObj.setEdgeId(result.get<int>(DB_FIELD_PT_EDGES_EDGE_ID));
	outObj.setWaitTime(result.get<double>(DB_FIELD_PT_EDGES_WAIT_TIME));
	outObj.setWalkTime()(result.get<double>(DB_FIELD_PT_EDGES_WALK_TIME));
	outObj.setTransitTime(result.get<double>(DB_FIELD_PT_EDGES_TRANSIT_TIME));
	outObj.setTransferPenalty(result.get<double>(DB_FIELD_PT_EDGES_TRANSFER_PENALTY));
	outObj.setTransitTime(result.get<double>(DB_FIELD_PT_EDGES_DAY_TRANSIT_TIME));
	outObj.setDist(result.get<double>(DB_FIELD_PT_EDGES_DIST));

}

void Pt_EdgesSqlDao::toRow(pt_network_edges& data, Parameters& outParams, bool update) {
}
