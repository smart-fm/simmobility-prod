//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   BuildingDao.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on May 7, 2013, 3:59 PM
 */

#include "BuildingDao.hpp"
#include "DatabaseHelper.hpp"

using namespace sim_mob::db;
using namespace sim_mob::long_term;

BuildingDao::BuildingDao(DB_Connection& connection)
: SqlAbstractDao<Building>(connection, DB_TABLE_BUILDING,
DB_INSERT_BUILDING, DB_UPDATE_BUILDING, DB_DELETE_BUILDING,
DB_GETALL_BUILDING, DB_GETBYID_BUILDING) {}

BuildingDao::~BuildingDao() {
}

void BuildingDao::fromRow(Row& result, Building& outObj) {
    outObj.id = result.get<BigSerial>(DB_FIELD_ID, INVALID_ID);
    outObj.typeId = result.get<BigSerial>(DB_FIELD_TYPE_ID, INVALID_ID);
    outObj.parcelId = result.get<BigSerial>(DB_FIELD_PARCEL_ID, INVALID_ID);
    outObj.tenureId = result.get<BigSerial>(DB_FIELD_TENURE_ID, INVALID_ID);
    outObj.builtYear = result.get<int>(DB_FIELD_BUILT_YEAR, 0);
    outObj.storeys = result.get<int>(DB_FIELD_STOREYS, 0);
    outObj.parkingSpaces = result.get<int>(DB_FIELD_PARKING_SPACES, 0);
    outObj.landedArea = result.get<double>(DB_FIELD_LANDED_AREA, 0);
}

void BuildingDao::toRow(Building& data, Parameters& outParams, bool update) {
}
