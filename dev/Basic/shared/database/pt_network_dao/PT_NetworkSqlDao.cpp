//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "PT_NetworkSqlDao.hpp"

#include <string>
#include <stdexcept>
#include "PT_DatabaseHelper.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"

using namespace sim_mob;
using namespace sim_mob::db;

PT_VerticesSqlDao::PT_VerticesSqlDao(DB_Connection& connection, std::string query) :
		SqlAbstractDao<PT_NetworkVertex>(connection, "", "", "", "", query, "")
{
}

PT_VerticesSqlDao::~PT_VerticesSqlDao()
{
}

void PT_VerticesSqlDao::fromRow(Row& result, PT_NetworkVertex& outObj)
{
	outObj.setStopId(result.get<std::string>(DB_FIELD_PT_VERTICES_STOP_ID, EMPTY_STRING));
	outObj.setStopCode(result.get<std::string>(DB_FIELD_PT_VERTICES_STOP_CODE, EMPTY_STRING));
	outObj.setStopName(result.get<std::string>(DB_FIELD_PT_VERTICES_STOP_NAME, EMPTY_STRING));
	outObj.setStopLatitude(result.get<double>(DB_FIELD_PT_VERTICES_STOP_LATITUDE));
	outObj.setStopLongitude(result.get<double>(DB_FIELD_PT_VERTICES_STOP_LONGITUDE));
	outObj.setEzlinkName(result.get<std::string>(DB_FIELD_PT_VERTICES_EZLINK_NAME, EMPTY_STRING));
	outObj.setStopType(result.get<int>(DB_FIELD_PT_VERTICES_STOP_TYPE));
	outObj.setStopDesc(result.get<std::string>(DB_FIELD_PT_VERTICES_STOP_DESCRIPTION, EMPTY_STRING));
}

void PT_VerticesSqlDao::toRow(PT_NetworkVertex& data, Parameters& outParams, bool update)
{
	throw std::runtime_error("Function PT_VerticesSqlDao::toRow not implemented");
}

Pt_EdgesSqlDao::Pt_EdgesSqlDao(DB_Connection& connection, std::string query) :
		SqlAbstractDao<PT_NetworkEdge>(connection, "", "", "", "", query, "")
{
}

Pt_EdgesSqlDao::~Pt_EdgesSqlDao()
{
}

void Pt_EdgesSqlDao::fromRow(Row& result, PT_NetworkEdge& outObj)
{
	outObj.setStartStop(result.get<std::string>(DB_FIELD_PT_EDGES_START_STOP, EMPTY_STRING));
	outObj.setEndStop(result.get<std::string>(DB_FIELD_PT_EDGES_END_STOP, EMPTY_STRING));
	outObj.setType(result.get<std::string>(DB_FIELD_PT_EDGES_R_TYPE, EMPTY_STRING));
	outObj.setServiceLine(result.get<std::string>(DB_FIELD_PT_SERVICE_LINE, EMPTY_STRING));
	outObj.setRoadIndex(result.get<std::string>(DB_FIELD_PT_EDGES_ROAD_INDEX, EMPTY_STRING));
	outObj.setRoadEdgeId(result.get<std::string>(DB_FIELD_PT_EDGES_ROAD_EDGE_ID, EMPTY_STRING));
	outObj.setServiceLines(result.get<std::string>(DB_FIELD_PT_EDGES_R_SERVICE_LINES, EMPTY_STRING));
	outObj.setLinkTravelTimeSecs(result.get<double>(DB_FIELD_PT_EDGES_LINK_TRAVEL_TIME));
	outObj.setEdgeId(result.get<int>(DB_FIELD_PT_EDGES_EDGE_ID));
	outObj.setWaitTimeSecs(result.get<double>(DB_FIELD_PT_EDGES_WAIT_TIME));
	outObj.setWalkTimeSecs(result.get<double>(DB_FIELD_PT_EDGES_WALK_TIME));
	outObj.setTransitTimeSecs(result.get<double>(DB_FIELD_PT_EDGES_TRANSIT_TIME));
	outObj.setTransferPenaltySecs(result.get<double>(DB_FIELD_PT_EDGES_TRANSFER_PENALTY));
	outObj.setDayTransitTimeSecs(result.get<double>(DB_FIELD_PT_EDGES_DAY_TRANSIT_TIME));
	outObj.setDistKms(result.get<double>(DB_FIELD_PT_EDGES_DIST));

}

void Pt_EdgesSqlDao::toRow(PT_NetworkEdge& data, Parameters& outParams, bool update)
{
	throw std::runtime_error("Function Pt_EdgesSqlDao::toRow not implemented");
}

