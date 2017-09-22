//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License";" as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <string>

namespace sim_mob
{
/**
 * Fields for public transit vertices database
 */
const std::string EMPTY_STRING = std::string();
const std::string DB_FIELD_PT_VERTICES_STOP_ID = "stop_id";
const std::string DB_FIELD_PT_VERTICES_STOP_CODE = "stop_code";
const std::string DB_FIELD_PT_VERTICES_STOP_NAME = "stop_name";
const std::string DB_FIELD_PT_VERTICES_STOP_LATITUDE = "stop_lat";
const std::string DB_FIELD_PT_VERTICES_STOP_LONGITUDE = "stop_lon";
const std::string DB_FIELD_PT_VERTICES_EZLINK_NAME = "ezlink_name";
const std::string DB_FIELD_PT_VERTICES_STOP_TYPE = "stop_type";
const std::string DB_FIELD_PT_VERTICES_STOP_DESCRIPTION = "stop_desc";
/**
 * Fields for public transit Edges database
 */
const std::string DB_FIELD_PT_EDGES_START_STOP = "start_stop";
const std::string DB_FIELD_PT_EDGES_END_STOP = "end_stop";
const std::string DB_FIELD_PT_EDGES_R_TYPE = "r_type";
const std::string DB_FIELD_PT_SERVICE_LINE = "service_line";
const std::string DB_FIELD_PT_EDGES_ROAD_INDEX = "road_index";
const std::string DB_FIELD_PT_EDGES_ROAD_EDGE_ID = "road_edge_id";
const std::string DB_FIELD_PT_EDGES_R_SERVICE_LINES = "r_service_lines";
const std::string DB_FIELD_PT_EDGES_LINK_TRAVEL_TIME = "link_travel_time";
const std::string DB_FIELD_PT_EDGES_EDGE_ID = "edge_id";
const std::string DB_FIELD_PT_EDGES_WAIT_TIME = "wait_time";
const std::string DB_FIELD_PT_EDGES_WALK_TIME = "walk_time";
const std::string DB_FIELD_PT_EDGES_TRANSIT_TIME = "transit_time";
const std::string DB_FIELD_PT_EDGES_TRANSFER_PENALTY = "transfer_penalty";
const std::string DB_FIELD_PT_EDGES_DAY_TRANSIT_TIME = "day_transit_time";
const std::string DB_FIELD_PT_EDGES_DIST = "dist";

} // end namespace sim_mob
