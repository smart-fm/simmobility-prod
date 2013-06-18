/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   IndividualDao.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on May 7, 2013, 3:59 PM
 */

#include "IndividualDao.hpp"
#include "DatabaseHelper.h"

using namespace sim_mob;
using namespace sim_mob::long_term;

IndividualDao::IndividualDao(DBConnection* connection)
: AbstractDao<Individual>(connection, DB_TABLE_INDIVIDUAL,
DB_INSERT_INDIVIDUAL, DB_UPDATE_INDIVIDUAL, DB_DELETE_INDIVIDUAL,
DB_GETALL_INDIVIDUAL, DB_GETBYID_INDIVIDUAL) {
    fromRowCallback = DAO_FROM_ROW_CALLBACK_HANDLER(Individual, IndividualDao::FromRow);
    toRowCallback = DAO_TO_ROW_CALLBACK_HANDLER(Individual, IndividualDao::ToRow);
}

IndividualDao::~IndividualDao() {
}

void IndividualDao::FromRow(Row& result, Individual& outObj) {
    outObj.id = result.get<int>(DB_FIELD_PERSON_ID);
    outObj.householdId = result.get<int>(DB_FIELD_HOUSEHOLD_ID);
    outObj.age = result.get<int>(DB_FIELD_AGE);
    outObj.employmentStatus = ToEmploymentStatus(result.get<int>(DB_FIELD_EMPLOYMENT_STATUS));
    outObj.hboTrips = result.get<int>(DB_FIELD_HBO_TRIPS);
    outObj.hbwTrips = result.get<int>(DB_FIELD_HBW_TRIPS);
    outObj.income = (float) result.get<int>(DB_FIELD_EARNING);
    outObj.jobId = result.get<int>(DB_FIELD_JOB_ID);
    outObj.numberOfCars = result.get<int>(DB_FIELD_CARS);
    outObj.race = ToRace(result.get<int>(DB_FIELD_RACE_ID));
    outObj.sex = ToSex(result.get<int>(DB_FIELD_SEX));
    outObj.transportMode = result.get<int>(DB_FIELD_MODE);
    outObj.workAtHome = (result.get<int>(DB_FIELD_WORK_AT_HOME) == 0) ? false : true;
    outObj.zoneId = result.get<int>(DB_FIELD_ZONE_ID);
}

void IndividualDao::ToRow(Individual& data, Parameters& outParams, bool update) {
}