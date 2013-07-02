/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   BuildingDao.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on May 7, 2013, 3:59 PM
 */

#include "BuildingDao.hpp"
#include "DatabaseHelper.hpp"

using namespace sim_mob;
using namespace sim_mob::long_term;

BuildingDao::BuildingDao(DBConnection* connection)
: AbstractDao<Building>(connection, DB_TABLE_BUILDING,
DB_INSERT_BUILDING, DB_UPDATE_BUILDING, DB_DELETE_BUILDING,
DB_GETALL_BUILDING, DB_GETBYID_BUILDING) {
    fromRowCallback = DAO_FROM_ROW_CALLBACK_HANDLER(Building, BuildingDao::FromRow);
    toRowCallback = DAO_TO_ROW_CALLBACK_HANDLER(Building, BuildingDao::ToRow);
}

BuildingDao::~BuildingDao() {
}

void BuildingDao::FromRow(Row& result, Building& outObj) {
    outObj.id = result.get<BigSerial>(DB_FIELD_ID);
    outObj.typeId = result.get<BigSerial>(DB_FIELD_TYPE_ID);
    outObj.projectId = result.get<BigSerial>(DB_FIELD_PROJECT_ID);
    outObj.builtYear = result.get<int>(DB_FIELD_BUILT_YEAR);
    outObj.floorArea = result.get<double>(DB_FIELD_FLOOR_AREA);
    outObj.storeys = result.get<int>(DB_FIELD_STOREYS);
    outObj.parkingSpaces = result.get<int>(DB_FIELD_PARKING_SPACES);
}

void BuildingDao::ToRow(Building& data, Parameters& outParams, bool update) {
}