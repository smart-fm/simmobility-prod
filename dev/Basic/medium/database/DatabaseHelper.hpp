//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License";" as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

namespace sim_mob
{
namespace medium
{
/**
 * Fields for mongoDB population data
 */
const std::string MONGO_FIELD_ID = "_id";
const std::string MONGO_FIELD_HOUSEHOLD_ID = "hhid";
const std::string MONGO_FIELD_HH_FACTOR = "hhfactor";
const std::string MONGO_FIELD_INCOME_ID = "income_id";
const std::string MONGO_FIELD_PERSON_TYPE_ID = "person_type_id";
const std::string MONGO_FIELD_AGE_CATEGORY_ID = "age_id";
const std::string MONGO_FIELD_WORK_AT_HOME = "work_from_home_dummy";
const std::string MONGO_FIELD_DRIVER_LICENCE = "has_driving_license";
const std::string MONGO_FIELD_STUDENT_TYPE_ID = "student_type_id";
const std::string MONGO_FIELD_UNIVERSITY_STUDENT = "universitystudent";
const std::string MONGO_FIELD_FEMALE = "female_dummy";
const std::string MONGO_FIELD_HOME_MTZ = "home_mtz";
const std::string MONGO_FIELD_WORK_MTZ = "fix_work_location_mtz";
const std::string MONGO_FIELD_SCHOOL_MTZ = "school_location_mtz";
const std::string MONGO_FIELD_HH_ONLY_ADULTS = "only_adults";
const std::string MONGO_FIELD_HH_ONLY_WORKERS = "only_workers";
const std::string MONGO_FIELD_HH_NUM_UNDER_4 = "num_underfour";
const std::string MONGO_FIELD_HH_UNDER_15 = "presence_of_under15";
const std::string MONGO_FIELD_CAR_OWN = "car_own";
const std::string MONGO_FIELD_CAR_OWN_NORMAL = "car_own_normal";
const std::string MONGO_FIELD_CAR_OWN_OFFPEAK = "car_own_offpeak";
const std::string MONGO_FIELD_MOTOR_OWN = "motor_own";
const std::string MONGO_FIELD_MISSING_INCOME = "missingincome";
const std::string MONGO_FIELD_WORK_TIME_FLEX = "worktime_flex";
const std::string MONGO_FIELD_WORK_LOGSUM = "worklogsum";
const std::string MONGO_FIELD_EDU_LOGSUM = "edulogsum";
const std::string MONGO_FIELD_SHOP_LOGSUM = "shoplogsum";
const std::string MONGO_FIELD_OTHER_LOGSUM = "otherlogsum";
const std::string MONGO_FIELD_DPT_LOGSUM = "dptlogsum";
const std::string MONGO_FIELD_DPS_LOGSUM = "dpslogsum";

/**
 * Fields for mongoDB Zone data
 */
const std::string MONGO_FIELD_ZONE_ID = "zone_id";
const std::string MONGO_FIELD_ZONE_CODE = "zone_code";
const std::string MONGO_FIELD_ZONE_SHOPS = "shop";
const std::string MONGO_FIELD_ZONE_PARKING_RATE = "parking_rate";
const std::string MONGO_FIELD_ZONE_RESIDENT_WORKERS = "resident_workers";
const std::string MONGO_FIELD_ZONE_CENTRAL_ZONE = "central_dummy";
const std::string MONGO_FIELD_ZONE_EMPLOYMENT = "employment";
const std::string MONGO_FIELD_ZONE_POPULATION = "population";
const std::string MONGO_FIELD_ZONE_AREA = "area";
const std::string MONGO_FIELD_ZONE_TOTAL_ENROLLMENT = "total_enrollment";
const std::string MONGO_FIELD_ZONE_RESIDENT_STUDENTS = "resident_students";
const std::string MONGO_FIELD_ZONE_CBD_ZONE = "cbd_dummy";

/**
 * Fields for MongoDB cost data
 */
const std::string MONGO_FIELD_COST_ORIGIN = "origin";
const std::string MONGO_FIELD_COST_DESTINATION = "destin";
const std::string MONGO_FIELD_COST_PUB_WTT = "pub_wtt";
const std::string MONGO_FIELD_COST_CAR_IVT = "car_ivt";
const std::string MONGO_FIELD_COST_PUB_OUT = "pub_out";
const std::string MONGO_FIELD_COST_PUB_WALKT = "pub_walkt";
const std::string MONGO_FIELD_COST_DISTANCE = "distance";
const std::string MONGO_FIELD_COST_CAR_ERP = "car_cost_erp";
const std::string MONGO_FIELD_COST_PUB_IVT = "pub_ivt";
const std::string MONGO_FIELD_COST_AVG_TRANSFER = "avg_transfer";
const std::string MONGO_FIELD_COST_PUB_COST = "pub_cost";

/**
 * Fields from MongoDB zone_aimsunnode_mapping data
 */
const std::string MONGO_FIELD_NODE_ID = "node_id";
const std::string MONGO_FIELD_MTZ_1092 = "MTZ_1092";
const std::string MONGO_FIELD_MTZ = "MTZ";
const std::string MONGO_FIELD_SOURCE_NODE = "source";
const std::string MONGO_FIELD_SINK_NODE = "sink";
const std::string MONGO_FIELD_BUS_TERMINUS_NODE = "bus_terminus_node";

/**
 * Fields for MTZ1169 to MTZ1092 mapping data
 */
const std::string MONGO_FIELD_MTZ1092 = "MTZ1092";

} // end namespace medium
} // end namespace sim_mob
