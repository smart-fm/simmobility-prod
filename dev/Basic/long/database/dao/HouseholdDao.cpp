/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   HouseholdDao.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on April 23, 2013, 5:17 PM
 */

#include "HouseholdDao.hpp"
#include "DatabaseHelper.hpp"

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
    outObj.id = result.get<BigSerial>(DB_FIELD_ID, INVALID_ID);
    outObj.unitId = result.get<BigSerial>(DB_FIELD_UNIT_ID, INVALID_ID);
    outObj.size = result.get<int>(DB_FIELD_SIZE, 0);
    outObj.children = result.get<int>(DB_FIELD_CHILDREN, 0);
    outObj.income = result.get<double>(DB_FIELD_INCOME, 0);
    outObj.carOwnership = result.get<double>(DB_FIELD_CAR_OWNERSHIP, 0);
    outObj.housingDuration = result.get<int>(DB_FIELD_HOUSING_DURATION, 0);
}

void HouseholdDao::ToRow(Household& data, Parameters& outParams, bool update) {
    outParams.push_back(data.unitId);
    outParams.push_back(data.size);
    outParams.push_back(data.children);
    outParams.push_back(data.income);
    outParams.push_back(data.carOwnership);
    outParams.push_back(data.housingDuration);
    if (update) {
        outParams.push_back(data.id);
    } else {// if is insert we need to put the id on the beginning.
        outParams.insert(outParams.begin(), data.id);
    }
}