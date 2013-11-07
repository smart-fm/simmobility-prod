//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * File:   UnitTypeDao.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on July 1, 2013, 3:59 PM
 */

#include "UnitTypeDao.hpp"
#include "DatabaseHelper.hpp"

using namespace sim_mob::db;
using namespace sim_mob::long_term;
using std::string;

UnitTypeDao::UnitTypeDao(DBConnection* connection)
: AbstractDao<UnitType>(connection, DB_TABLE_UNIT_TYPE,
DB_INSERT_UNIT_TYPE, DB_UPDATE_UNIT_TYPE, DB_DELETE_UNIT_TYPE,
DB_GETALL_UNIT_TYPE, DB_GETBYID_UNIT_TYPE) {}

UnitTypeDao::~UnitTypeDao() {
}

void UnitTypeDao::fromRow(Row& result, UnitType& outObj) {
    outObj.id = result.get<BigSerial>(DB_FIELD_ID, INVALID_ID);
    outObj.name = result.get<string>(DB_FIELD_NAME, EMPTY_STR);
    outObj.type = result.get<int> (DB_FIELD_TYPE, 0);
}

void UnitTypeDao::toRow(UnitType& data, Parameters& outParams, bool update) {
}
