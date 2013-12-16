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
	 * Views for long-term population database
	 */
	const std::string DB_VIEW_PREDAY_PERSON = APPLY_SCHEMA(MAIN_SCHEMA, "preday_person");

	/**
	 * Fields for long-term population database
	 */
	const std::string DB_FIELD_ID = "id";
	const std::string DB_FIELD_HOUSEHOLD_ID = "household_id";
	const std::string DB_FIELD_INCOME = "income";
	const std::string DB_FIELD_EMP_STATUS_ID = "employment_status_id";
	const std::string DB_FIELD_AGE_CATEGORY_ID = "age_category_id";
	const std::string DB_FIELD_WORK_AT_HOME = "work_at_home";
	const std::string DB_FIELD_DRIVER_LICENCE = "driver_license";
	const std::string DB_FIELD_UNIV_STUDENT = "university_student";
	const std::string DB_FIELD_FEMALE = "female";
	const std::string DB_FIELD_HOME_PCODE = "home_postcode";
	const std::string DB_FIELD_HOME_MTZ = "home_mtz";
	const std::string DB_FIELD_WORK_PCODE = "work_postcode";
	const std::string DB_FIELD_WORK_MTZ = "work_mtz";
	const std::string DB_FIELD_HH_ONLY_ADULTS = "hh_only_adults";
	const std::string DB_FIELD_HH_ONLY_WORKERS = "hh_only_workers";
	const std::string DB_FIELD_HH_NUM_UNDER_4 = "hh_num_under4";
	const std::string DB_FIELD_HH_NUM_UNDER_15 = "hh_num_under15";
	const std::string DB_FIELD_CAR_OWN_NORMAL = "car_own_normal";
	const std::string DB_FIELD_CAR_OWN_OFFPEAK = "car_own_offpeak";
	const std::string DB_FIELD_MOTOR_OWN = "motor_own";

    /**
     * GET ALL for long-term population database
     */
	const std::string DB_GETALL_PREDAY_PERSON = "SELECT * FROM " + DB_VIEW_PREDAY_PERSON + LIMIT;

	/**
	 * Fields for mongoDB population data
	 */
	const std::string MONGO_FIELD_ID = "_id";
	const std::string MONGO_FIELD_HOUSEHOLD_ID = "hhid";
	const std::string MONGO_FIELD_INCOME_ID = "income_id";
	const std::string MONGO_FIELD_PERSON_TYPE_ID = "person_type_id";
	const std::string MONGO_FIELD_AGE_CATEGORY_ID = "age_id";
	const std::string MONGO_FIELD_WORK_AT_HOME = "work_from_home_dummy";
	const std::string MONGO_FIELD_DRIVER_LICENCE = "has_driving_license";
	const std::string MONGO_FIELD_STUDENT_TYPE_ID = "student_type_id";
	const std::string MONGO_FIELD_FEMALE = "female_dummy";
	const std::string MONGO_FIELD_HOME_MTZ = "home_mtz";
	const std::string MONGO_FIELD_WORK_MTZ = "fix_work_location_mtz";
	const std::string MONGO_FIELD_SCHOOL_MTZ = "school_location_mtz";
	const std::string MONGO_FIELD_HH_ONLY_ADULTS = "only_adults";
	const std::string MONGO_FIELD_HH_ONLY_WORKERS = "only_workers";
	const std::string MONGO_FIELD_HH_NUM_UNDER_4 = "num_underfour";
	const std::string MONGO_FIELD_HH_UNDER_15 = "num_not_eligible";
	const std::string MONGO_FIELD_CAR_OWN_NORMAL = "car_own_normal";
	const std::string MONGO_FIELD_CAR_OWN_OFFPEAK = "car_own_offpeak";
	const std::string MONGO_FIELD_MOTOR_OWN = "motor_own";

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

} // end namespace medium
} // end namespace sim_mob
