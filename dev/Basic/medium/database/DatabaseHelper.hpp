//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License";" as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * DatabaseHelper.hpp
 *
 *  Created on: Nov 18, 2013
 *      Author: Harish Loganathan
 */

#pragma once

namespace sim_mob {
namespace medium {
	#define APPLY_SCHEMA(schema, field) std::string(schema)+std::string(field)
	#define APPLY_BRACKETS(arg) std::string("(")+std::string(arg)+std::string(")")

	/**
	 * Useful string constants
	 */
	const std::string EMPTY_STRING = "";

    /**
     * Schemas
     */
    const std::string MAIN_SCHEMA = "main.";
    const std::string PUBLIC_SCHEMA = "public.";

    /**
     * Tables
     */
    const std::string DB_TABLE_INCOME_CATEGORIES = APPLY_SCHEMA(MAIN_SCHEMA, "income_category");
    const std::string DB_TABLE_VEHICLE_CATEGORIES = APPLY_SCHEMA(MAIN_SCHEMA, "vehicle_category");
    const std::string DB_TABLE_LOGSUMS = APPLY_SCHEMA(PUBLIC_SCHEMA, "preday_logsum");

	/**
	 * Stored procedures for long-term population database
	 */
	const std::string DB_SP_GET_INDIVIDUAL_IDS = APPLY_SCHEMA(MAIN_SCHEMA, "getindividualids");
	const std::string DB_SP_GET_INDIVIDUAL_BY_ID_FOR_PREDAY = APPLY_SCHEMA(MAIN_SCHEMA, "getindividualbyidforpreday");
	const std::string DB_SP_GET_ADDRESS_TAZ = APPLY_SCHEMA(MAIN_SCHEMA, "getaddresstaz");

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
	const std::string DB_FIELD_WORK_LOGSUM = "worklogsum";
	const std::string DB_FIELD_EDU_LOGSUM = "edulogsum";
	const std::string DB_FIELD_SHOP_LOGSUM = "shoplogsum";
	const std::string DB_FIELD_OTHER_LOGSUM = "otherlogsum";
	const std::string DB_FIELD_DPT_LOGSUM = "dptlogsum";
	const std::string DB_FIELD_DPS_LOGSUM = "dpslogsum";

	const std::string DB_FIELD_INCOME_CATEGORY_LOWER_LIMIT = "low_limit";
	const std::string DB_FIELD_VEHICLE_CATEGORY_NAME = "name";

	const std::string SEARCH_STRING_CAR_OWN_NORMAL = "car (normal time)";
	const std::string SEARCH_STRING_CAR_OWN_OFF_PEAK = "car (off peak time)";
	const std::string SEARCH_STRING_MOTORCYCLE = "motorcycle";

    /** get all individual ids from long-term population database */
	const std::string DB_GET_ALL_PERSON_IDS = "SELECT * FROM " + DB_SP_GET_INDIVIDUAL_IDS + APPLY_BRACKETS();

	/** load a specific individual by id */
	const std::string DB_GET_PERSON_BY_ID = "SELECT * FROM " + DB_SP_GET_INDIVIDUAL_BY_ID_FOR_PREDAY; //argument to be passed

	/** load address taz mapping from LT database */
	const std::string DB_GET_ADDRESS_TAZ = "SELECT * FROM " + DB_SP_GET_ADDRESS_TAZ + APPLY_BRACKETS();

	/** load income categories */
	const std::string DB_GET_INCOME_CATEGORIES = "SELECT * FROM " + DB_TABLE_INCOME_CATEGORIES;

	/** load vehicle categories */
	const std::string DB_GET_VEHICLE_CATEGORIES = "SELECT * FROM " + DB_TABLE_VEHICLE_CATEGORIES;


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
	const std::string MONGO_FIELD_NODE_ID = "_id";
	const std::string MONGO_FIELD_MTZ = "MTZ_1092";
	const std::string MONGO_FIELD_SOURCE_NODE = "source";
	const std::string MONGO_FIELD_SINK_NODE = "sink";

} // end namespace medium
} // end namespace sim_mob
