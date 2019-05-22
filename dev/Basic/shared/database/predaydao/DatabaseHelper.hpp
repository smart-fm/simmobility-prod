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
const std::string DB_FIELD_V_ID = "vehicle_id";
const std::string DB_FIELD_V_DRIVETRAIN = "vehicle_drivetrain";
const std::string DB_FIELD_V_MAKE = "vehicle_make";
const std::string DB_FIELD_V_MODEL = "vehicle_model";

/**
 * Logsum fields
 */
const std::string DB_FIELD_WORK_LOGSUM = "work";
const std::string DB_FIELD_EDUCATION_LOGSUM = "education";
const std::string DB_FIELD_SHOP_LOGSUM = "shop";
const std::string DB_FIELD_OTHER_LOGSUM = "other";
const std::string DB_FIELD_DPT_LOGSUM = "dp_tour";
const std::string DB_FIELD_DPS_LOGSUM = "dp_stop";
const std::string DB_FIELD_DPB_LOGSUM = "dp_binary";

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
const std::string DB_FIELD_COST_SMS_WTT = "wait_sms";
const std::string DB_FIELD_COST_SMS_POOL_WTT = "wait_sms_pool";
const std::string DB_FIELD_COST_AMOD_WTT = "wait_amod";
const std::string DB_FIELD_COST_AMOD_POOL_WTT = "wait_amod_pool";
const std::string DB_FIELD_COST_AMOD_MINIBUS_WTT = "wait_amod_minibus";

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

const std::string DB_FIELD_SUPPLY_NODE_ID = "id";

const std::string DB_FIELD_ZONE_WITHOUT_NODE = "taz";


} // end namespace sim_mob
