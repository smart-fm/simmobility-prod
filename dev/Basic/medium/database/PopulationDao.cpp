//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * PredayDao.cpp
 *
 *  Created on: Nov 15, 2013
 *      Author: Harish Loganathan
 */

#include "PopulationDao.hpp"

#include "DatabaseHelper.hpp"

using namespace sim_mob;
using namespace sim_mob::db;
using namespace sim_mob::medium;

PopulationDao::PopulationDao(DB_Connection& connection)
: AbstractDao<PersonParams>(connection, DB_VIEW_PREDAY_PERSON, "", "", "", DB_GETALL_PREDAY_PERSON, "")
{}

PopulationDao::~PopulationDao()
{}

void PopulationDao::fromRow(Row& result, PersonParams& outObj) {
	outObj.setPersonId(result.get<int>(DB_FIELD_ID));
	outObj.setIncomeId(result.get<int>(DB_FIELD_INCOME));
	outObj.setPersonTypeId(result.get<int>(DB_FIELD_EMP_STATUS_ID));
	outObj.setAgeId(result.get<int>(DB_FIELD_AGE_CATEGORY_ID));
	outObj.setWorksAtHome(result.get<int>(DB_FIELD_WORK_AT_HOME));
	outObj.setHasDrivingLicence(result.get<int>(DB_FIELD_DRIVER_LICENCE));
	outObj.setIsUniversityStudent(result.get<int>(DB_FIELD_UNIV_STUDENT));
	outObj.setIsFemale(result.get<int>(DB_FIELD_FEMALE));
	outObj.setHomeLocation(result.get<int>(DB_FIELD_HOME_MTZ));
	outObj.setFixedWorkLocation(result.get<int>(DB_FIELD_WORK_MTZ));
	outObj.setHH_OnlyAdults(result.get<int>(DB_FIELD_HH_ONLY_ADULTS));
	outObj.setHH_OnlyWorkers(result.get<int>(DB_FIELD_HH_ONLY_WORKERS));
	outObj.setHH_NumUnder4(result.get<int>(DB_FIELD_HH_NUM_UNDER_4));
	outObj.setHH_NumUnder15(result.get<int>(DB_FIELD_HH_NUM_UNDER_15));
	outObj.setCarOwnNormal(result.get<int>(DB_FIELD_CAR_OWN_NORMAL));
	outObj.setCarOwnOffpeak(result.get<int>(DB_FIELD_CAR_OWN_OFFPEAK));
	outObj.setMotorOwn(result.get<int>(DB_FIELD_MOTOR_OWN));
}

void PopulationDao::toRow(PersonParams& data, Parameters& outParams, bool update) {
}
