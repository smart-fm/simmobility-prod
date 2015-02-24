//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License";" as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * DatabaseHelper.hpp
 *
 *  Created on: Feb 24, 2015
 *      Author: Prabhuraj
 */

#pragma once

namespace sim_mob {
namespace medium {
	#define APPLY_SCHEMA(schema, field) std::string(schema)+std::string(field)

	/**
	 * Useful string constants
	 */
	const std::string EMPTY_STRING = "";
    const std::string LIMIT_10 = " limit 10";
    const std::string LIMIT = LIMIT_10;

    /**
     * Schemas for long-term population database
     */
    const std::string MAIN_SCHEMA = "main.";

    /**
     * Tables
     */
    const std::string DB_TABLE_PREDAY_FLAT_TRIPCHAINS = "preday_trip_chains_flat";

	/**
	 * Views for long-term population database
	 */
	const std::string DB_VIEW_PREDAY_PERSON = APPLY_SCHEMA(MAIN_SCHEMA, "preday_person");

	/**
	 * Fields for public transit vertices database
	 */

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

} // end namespace medium
} // end namespace sim_mob
