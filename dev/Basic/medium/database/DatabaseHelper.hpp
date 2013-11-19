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
	 * Schemas
	 */
	const std::string EMPTY_STRING = "";
	const std::string MAIN_SCHEMA = "main.";
    const std::string LIMIT_10 = " limit 10";
    const std::string LIMIT = LIMIT_10;

	/**
	 * Views
	 */
	const std::string DB_VIEW_PREDAY_PERSON = APPLY_SCHEMA(MAIN_SCHEMA, "preday_person");

	/**
	 * Fields
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
     * GET ALL
     */
	const std::string DB_GETALL_PREDAY_PERSON = "SELECT * FROM " + DB_VIEW_PREDAY_PERSON + LIMIT;

} // end namespace medium
} // end namespace sim_mob
