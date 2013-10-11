//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   BuildingDao.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on May 7, 2013, 3:59 PM
 */

#include "UnitDao.hpp"
#include "DatabaseHelper.hpp"

using namespace sim_mob::db;
using namespace sim_mob::long_term;

UnitDao::UnitDao(DBConnection* connection)
: AbstractDao<Unit>(connection, DB_TABLE_UNIT,
DB_INSERT_UNIT, DB_UPDATE_UNIT, DB_DELETE_UNIT,
DB_GETALL_UNIT, DB_GETBYID_UNIT) {
    fromRowCallback = DAO_FROM_ROW_CALLBACK_HANDLER(Unit, UnitDao::FromRow);
    toRowCallback = DAO_TO_ROW_CALLBACK_HANDLER(Unit, UnitDao::ToRow);
}

UnitDao::~UnitDao() {
}

void UnitDao::FromRow(Row& result, Unit& outObj) {
    outObj.id = result.get<BigSerial>(DB_FIELD_ID, INVALID_ID);
    outObj.buildingId = result.get<BigSerial>(DB_FIELD_BUILDING_ID, INVALID_ID);
    outObj.typeId = result.get<BigSerial>(DB_FIELD_TYPE_ID, INVALID_ID);
    outObj.postcodeId = result.get<BigSerial>(DB_FIELD_POSTCODE_ID, INVALID_ID);
    outObj.storey = result.get<int>(DB_FIELD_STOREY, 0);
    outObj.floorArea = result.get<double>(DB_FIELD_FLOOR_AREA, 0);
    outObj.rent = result.get<double>(DB_FIELD_RENT, 0);
    outObj.available = false;
}

void UnitDao::ToRow(Unit& data, Parameters& outParams, bool update) {
}
