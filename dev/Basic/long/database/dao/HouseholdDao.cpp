//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   HouseholdDao.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on April 23, 2013, 5:17 PM
 */

#include "HouseholdDao.hpp"
#include "DatabaseHelper.hpp"

using namespace sim_mob::db;
using namespace sim_mob::long_term;

HouseholdDao::HouseholdDao(DB_Connection& connection)
: AbstractDao<Household>(connection, DB_TABLE_HOUSEHOLD,
DB_INSERT_HOUSEHOLD, DB_UPDATE_HOUSEHOLD, DB_DELETE_HOUSEHOLD,
DB_GETALL_HOUSEHOLD, DB_GETBYID_HOUSEHOLD) {
}

HouseholdDao::~HouseholdDao() {
}

void HouseholdDao::fromRow(Row& result, Household& outObj) {
    outObj.id = result.get<BigSerial>(DB_FIELD_ID, INVALID_ID);
    outObj.lifestyleId = result.get<BigSerial>(DB_FIELD_LIFESTYLE_ID, INVALID_ID);
    outObj.unitId = result.get<BigSerial>(DB_FIELD_UNIT_ID, INVALID_ID);
    outObj.ethnicityId = result.get<BigSerial>(DB_FIELD_ETHNICITY_ID, INVALID_ID);
    outObj.vehicleCategoryId = result.get<BigSerial>(DB_FIELD_VEHICLE_CATEGORY_ID, INVALID_ID);
    outObj.size = result.get<int>(DB_FIELD_SIZE, 0);
    outObj.children = result.get<int>(DB_FIELD_CHILDREN, 0);
    outObj.income = result.get<double>(DB_FIELD_INCOME, 0);
    outObj.housingDuration = result.get<int>(DB_FIELD_HOUSING_DURATION, 0);
    outObj.workers = result.get<int>(DB_FIELD_WORKERS, 0);
    outObj.ageOfHead = result.get<int>(DB_FIELD_AGE_OF_HEAD, 0);
}

void HouseholdDao::toRow(Household& data, Parameters& outParams, bool update) {
    /*outParams.push_back(data.unitId);
    outParams.push_back(data.size);
    outParams.push_back(data.children);
    outParams.push_back(data.income);
    outParams.push_back(data.housingDuration);
    if (update) {
        outParams.push_back(data.id);
    } else {// if is insert we need to put the id on the beginning.
        outParams.insert(outParams.begin(), data.id);
    }*/
}
