//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   BuildingDao.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on July 1, 2013, 3:59 PM
 */

#include "BuildingTypeDao.hpp"
#include "DatabaseHelper.hpp"

using namespace sim_mob::db;
using namespace sim_mob::long_term;
using std::string;

BuildingTypeDao::BuildingTypeDao(DBConnection* connection)
: AbstractDao<BuildingType>(connection, DB_TABLE_BUILDING_TYPE,
DB_INSERT_BUILDING_TYPE, DB_UPDATE_BUILDING_TYPE, DB_DELETE_BUILDING_TYPE,
DB_GETALL_BUILDING_TYPE, DB_GETBYID_BUILDING_TYPE) {
    fromRowCallback = DAO_FROM_ROW_CALLBACK_HANDLER(BuildingType, BuildingTypeDao::FromRow);
    toRowCallback = DAO_TO_ROW_CALLBACK_HANDLER(BuildingType, BuildingTypeDao::ToRow);
}

BuildingTypeDao::~BuildingTypeDao() {
}

void BuildingTypeDao::FromRow(Row& result, BuildingType& outObj) {
    outObj.id = result.get<BigSerial>(DB_FIELD_ID, INVALID_ID);
    outObj.name = result.get<string>(DB_FIELD_NAME, EMPTY_STR);
    outObj.type = result.get<int>(DB_FIELD_TYPE, 0);
}

void BuildingTypeDao::ToRow(BuildingType& data, Parameters& outParams, bool update) {
}
