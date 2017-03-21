//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License";" as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

namespace sim_mob
{
#define APPLY_SCHEMA(schema, field) std::string(schema)+std::string(field)

/**
 * Useful string constants
 */
const std::string EMPTY_STRING = "";

/**
 * Schemas
 */
const std::string MAIN_SCHEMA = "synpop12.";
const std::string CALIBRATION_SCHEMA = "calibration2012.";
const std::string PUBLIC_SCHEMA = "public.";
const std::string DEMAND_SCHEMA = "demand.";

/**
 * Tables
 */
const std::string DB_TABLE_INCOME_CATEGORIES = APPLY_SCHEMA(MAIN_SCHEMA, "income_category");
const std::string DB_TABLE_VEHICLE_OWNERSHIP_STATUS = APPLY_SCHEMA(MAIN_SCHEMA, "vehicle_ownership_status");
const std::string DB_TABLE_AM_COSTS = APPLY_SCHEMA(DEMAND_SCHEMA, "learned_amcosts");
const std::string DB_TABLE_PM_COSTS = APPLY_SCHEMA(DEMAND_SCHEMA, "learned_pmcosts");
const std::string DB_TABLE_OP_COSTS = APPLY_SCHEMA(DEMAND_SCHEMA, "learned_opcosts");
const std::string DB_TABLE_TAZ = APPLY_SCHEMA(DEMAND_SCHEMA, "taz_2012");
const std::string DB_TABLE_TCOST_PVT = APPLY_SCHEMA(DEMAND_SCHEMA, "learned_tcost_car");
const std::string DB_TABLE_TCOST_PT = APPLY_SCHEMA(DEMAND_SCHEMA, "learned_tcost_bus");
const std::string DB_TABLE_NODE_ZONE_MAP = APPLY_SCHEMA(DEMAND_SCHEMA, "node_taz_map");

/**
 * Stored procedures for long-term population database
 */
const std::string DB_SP_GET_INDIVIDUAL_IDS = APPLY_SCHEMA(MAIN_SCHEMA, "getindividualids()");
const std::string DB_SP_GET_INDIVIDUAL_BY_ID_FOR_PREDAY = APPLY_SCHEMA(MAIN_SCHEMA, "getindividualbyidforpreday(:_id)");
const std::string DB_SP_GET_ADDRESSES = APPLY_SCHEMA(MAIN_SCHEMA, "getaddresses()");
const std::string DB_SP_GET_POSTCODE_NODE_MAP = APPLY_SCHEMA(PUBLIC_SCHEMA, "get_postcode_node_map()");

/**
 * Fields for long-term population database
 */
const std::string DB_FIELD_ID = "id";
const std::string DB_FIELD_PERSON_TYPE_ID = "person_type_id";
const std::string DB_FIELD_GENDER_ID = "gender_id";
const std::string DB_FIELD_STUDENT_TYPE_ID = "education_type_id";
const std::string DB_FIELD_VEHICLE_CATEGORY_ID = "vehicle_category_id";
const std::string DB_FIELD_AGE_CATEGORY_ID = "age_category_id";
const std::string DB_FIELD_INCOME = "income";
const std::string DB_FIELD_WORK_AT_HOME = "work_at_home";
const std::string DB_FIELD_CAR_LICENSE = "car_license";
const std::string DB_FIELD_MOTOR_LICENSE = "motor_license";
const std::string DB_FIELD_VANBUS_LICENSE = "vanbus_license";
const std::string DB_FIELD_WORK_TIME_FLEX = "time_restriction";
const std::string DB_FIELD_IS_STUDENT = "is_student";
const std::string DB_FIELD_HAS_FIXED_WORK_PLACE = "fixed_workplace";
const std::string DB_FIELD_ACTIVITY_ADDRESS_ID = "activity_address_id";
const std::string DB_FIELD_HOUSEHOLD_ID = "hhid";
const std::string DB_FIELD_HOME_ADDRESS_ID = "home_address_id";
const std::string DB_FIELD_HH_SIZE = "size";
const std::string DB_FIELD_HH_CHILDREN_UNDER_4 = "child_under4";
const std::string DB_FIELD_HH_CHILDREN_UNDER_15 = "child_under15";
const std::string DB_FIELD_HH_ADULTS = "adult";
const std::string DB_FIELD_HH_WORKERS = "workers";

/**
 * Logsum fields
 */
const std::string DB_FIELD_WORK_LOGSUM = "work";
const std::string DB_FIELD_EDUCATION_LOGSUM = "education";
const std::string DB_FIELD_SHOP_LOGSUM = "shop";
const std::string DB_FIELD_OTHER_LOGSUM = "other";
const std::string DB_FIELD_DPT_LOGSUM = "dp_tour";
const std::string DB_FIELD_DPS_LOGSUM = "dp_stop";

const std::string DB_FIELD_INCOME_CATEGORY_LOWER_LIMIT = "low_limit";
const std::string DB_FIELD_VEHICLE_CATEGORY_NAME = "name";
const std::string DB_FIELD_ADDRESS_ID = "address_id";
const std::string DB_FIELD_TAZ_CODE = "taz_code";
const std::string DB_FIELD_POSTCODE = "postcode";
const std::string DB_FIELD_NODE_ID = "node_id";
const std::string DB_FIELD_DISTANCE_MRT = "distance_mrt";
const std::string DB_FIELD_DISTANCE_BUS = "distance_bus";
const std::string DB_FIELD_NUM_ADDRESSES = "num_addresses";

const std::string SEARCH_STRING_NO_VEHICLE = "No vehicle";
const std::string SEARCH_STRING_MULT_MOTORCYCLE_ONLY = "1+ Motor only";
const std::string SEARCH_STRING_ONE_CAR_OFF_PEAK_W_WO_MC= "1 Off-peak Car w/wo Motor";
const std::string SEARCH_STRING_ONE_NORMAL_CAR = "1 Normal Car only";
const std::string SEARCH_STRING_ONE_CAR_PLUS_MULT_MC = "1 Normal Car & 1+ Motor";
const std::string SEARCH_STRING_MULT_CAR_W_WO_MC = "2+ Normal Car w/wo Motor";

/**
 * Fields for Zone data (in postgres db)
 */
const std::string DB_FIELD_ZONE_ID = "zone_id";
const std::string DB_FIELD_ZONE_CODE = "zone_code";
const std::string DB_FIELD_ZONE_SHOPS = "shop";
const std::string DB_FIELD_ZONE_PARKING_RATE = "parking_rate";
const std::string DB_FIELD_ZONE_RESIDENT_WORKERS = "resident_workers";
const std::string DB_FIELD_ZONE_CENTRAL_ZONE = "central";
const std::string DB_FIELD_ZONE_EMPLOYMENT = "employment";
const std::string DB_FIELD_ZONE_POPULATION = "population";
const std::string DB_FIELD_ZONE_AREA = "area";
const std::string DB_FIELD_ZONE_TOTAL_ENROLLMENT = "total_enrollment";
const std::string DB_FIELD_ZONE_RESIDENT_STUDENTS = "resident_students";
const std::string DB_FIELD_ZONE_CBD_ZONE = "cbd";

/**
 * Fields for cost data (in postgres db)
 */
const std::string DB_FIELD_COST_ORIGIN = "origin_zone";
const std::string DB_FIELD_COST_DESTINATION = "destination_zone";
const std::string DB_FIELD_COST_PUB_WTT = "pub_wtt";
const std::string DB_FIELD_COST_CAR_IVT = "car_ivt";
const std::string DB_FIELD_COST_PUB_OUT = "pub_out";
const std::string DB_FIELD_COST_PUB_WALKT = "pub_walkt";
const std::string DB_FIELD_COST_DISTANCE = "distance";
const std::string DB_FIELD_COST_CAR_ERP = "car_cost_erp";
const std::string DB_FIELD_COST_PUB_IVT = "pub_ivt";
const std::string DB_FIELD_COST_AVG_TRANSFER = "avg_transfer";
const std::string DB_FIELD_COST_PUB_COST = "pub_cost";

/**
 * Fields for time dependent zone-zone travel times data (in postgres db)
 */
const std::string DB_FIELD_TCOST_ORIGIN = "origin_zone";
const std::string DB_FIELD_TCOST_DESTINATION = "destination_zone";
const std::string DB_FIELD_TCOST_INFO_UNAVAILABLE = "info_unavailable";
const std::string DB_FIELD_TCOST_TT_ARRIVAL_PREFIX = "tt_arrival_";
const std::string DB_FIELD_TCOST_TT_DEPARTURE_PREFIX = "tt_departure_";

/**
 * Fields for node to zone mapping data (in postgres db)
 */
const std::string DB_FIELD_NODE_TYPE = "node_type";
const std::string DB_FIELD_TRAFFIC_LIGHT = "traffic_light";
const std::string DB_FIELD_SOURCE = "source";
const std::string DB_FIELD_SINK = "sink";
const std::string DB_FIELD_EXPWAY = "expressway";
const std::string DB_FIELD_INTERSECT = "intersection";
const std::string DB_FIELD_BUS_TERMINUS = "bus_terminus_node";
const std::string DB_FIELD_TAZ = "taz";

/** get all individual ids from long-term population database */
const std::string DB_GET_ALL_PERSON_IDS = "SELECT * FROM " + DB_SP_GET_INDIVIDUAL_IDS;

/** load a specific individual by id */
const std::string DB_GET_PERSON_BY_ID = "SELECT * FROM " + DB_SP_GET_INDIVIDUAL_BY_ID_FOR_PREDAY; //argument to be passed

/** load address taz mapping from LT database */
const std::string DB_GET_ADDRESSES = "SELECT * FROM " + DB_SP_GET_ADDRESSES;

/** load postcode to simmobility node mapping */
const std::string DB_GET_POSTCODE_NODE_MAP = "SELECT * FROM " + DB_SP_GET_POSTCODE_NODE_MAP;

/** load income categories */
const std::string DB_GET_INCOME_CATEGORIES = "SELECT * FROM " + DB_TABLE_INCOME_CATEGORIES;

/** load vehicle categories */
const std::string DB_GET_VEHICLE_OWNERSHIP_STATUS = "SELECT * FROM " + DB_TABLE_VEHICLE_OWNERSHIP_STATUS;

/** load Costs */
const std::string DB_GET_ALL_AM_COSTS = "SELECT * FROM " + DB_TABLE_AM_COSTS;
const std::string DB_GET_ALL_PM_COSTS = "SELECT * FROM " + DB_TABLE_PM_COSTS;
const std::string DB_GET_ALL_OP_COSTS = "SELECT * FROM " + DB_TABLE_OP_COSTS;

/** load zone-zone tt data for a given OD zones **/
const std::string DB_GET_TCOST_PT_FOR_OD = "SELECT * FROM " + DB_TABLE_TCOST_PT +
		                                    " WHERE " + DB_FIELD_TCOST_ORIGIN + " = :origin"
		                                    "   AND " + DB_FIELD_TCOST_DESTINATION + " = :dest";

const std::string DB_GET_TCOST_PVT_FOR_OD = "SELECT * FROM " + DB_TABLE_TCOST_PVT +
		                                    " WHERE " + DB_FIELD_TCOST_ORIGIN + " = :origin"
		                                    "   AND " + DB_FIELD_TCOST_DESTINATION + " = :dest";

const std::string DB_GET_PUB_UNAVAILABLE_OD = "SELECT "+ DB_FIELD_TCOST_ORIGIN + ", " + DB_FIELD_TCOST_DESTINATION + " FROM " + DB_TABLE_TCOST_PT + " WHERE info_unavailable = TRUE";
const std::string DB_GET_PVT_UNAVAILABLE_OD = "SELECT "+ DB_FIELD_TCOST_ORIGIN + ", " + DB_FIELD_TCOST_DESTINATION + " FROM " + DB_TABLE_TCOST_PVT + " WHERE info_unavailable = TRUE";

/** load zones */
const std::string DB_GET_ALL_ZONES = "SELECT * FROM " + DB_TABLE_TAZ;

/** load node to zone mapping */
const std::string DB_GET_ALL_NODE_ZONE_MAP = "SELECT * FROM " + DB_TABLE_NODE_ZONE_MAP;

} // end namespace sim_mob
