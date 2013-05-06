/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   HouseholdDao.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on April 23, 2013, 5:17 PM
 */

#include "HouseholdDao.hpp"
#include "DatabaseHelper.h"

using namespace sim_mob;
using namespace sim_mob::long_term;

HouseholdDao::HouseholdDao(DBConnection* connection)
: AbstractDao<Household>(connection, DB_TABLE_HOUSEHOLD,
DB_INSERT_HOUSEHOLD, DB_UPDATE_HOUSEHOLD, DB_DELETE_HOUSEHOLD, DB_GETALL_HOUSEHOLD, DB_GETBYID_HOUSEHOLD) {
    fromRowCallback = DAO_FROM_ROW_CALLBACK_HANDLER(Household, HouseholdDao::FromRow);
    toRowCallback = DAO_TO_ROW_CALLBACK_HANDLER(Household, HouseholdDao::ToRow);
}

HouseholdDao::~HouseholdDao() {
}

void HouseholdDao::FromRow(Row& result, Household& outObj) {
    outObj.id = result.get<int>(DB_FIELD_HOUSEHOLD_ID);
    outObj.buildingId = result.get<int>(DB_FIELD_BUILDING_ID);
    outObj.numberOfMembers = result.get<int>(DB_FIELD_PERSONS);
    outObj.numberOfWorkers = result.get<int>(DB_FIELD_WORKERS);
    outObj.numberOfChildren = result.get<int>(DB_FIELD_CHILDREN);
    outObj.numberOfCars = result.get<int>(DB_FIELD_CARS);
    outObj.income = (float) result.get<int>(DB_FIELD_INCOME);
    outObj.headAge = result.get<int>(DB_FIELD_AGE_OF_HEAD);
    outObj.race = ToRace(result.get<int>(DB_FIELD_RACE_ID));
}

void HouseholdDao::ToRow(Household& data, Parameters& outParams, bool update) {
    outParams.push_back(data.buildingId);
    outParams.push_back(data.numberOfCars);
    outParams.push_back(data.numberOfWorkers);
    outParams.push_back(data.headAge);
    outParams.push_back(data.numberOfChildren);
    outParams.push_back((int) data.income);
    outParams.push_back(data.numberOfMembers);
    outParams.push_back((int) data.race);
    if (update){
        outParams.push_back(data.id);
    }else{// if is insert we need to put the id on the beginning.
        outParams.insert(outParams.begin(), data.id);
    }
}