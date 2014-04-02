//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   LandUseZoneDao.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on Mar 21, 2014, 5:17 PM
 */

#include "LandUseZoneDao.hpp"
#include "DatabaseHelper.hpp"

using namespace sim_mob::db;
using namespace sim_mob::long_term;

LandUseZoneDao::LandUseZoneDao(DB_Connection& connection)
: SqlAbstractDao<LandUseZone>(connection, DB_TABLE_LAND_USE_ZONE,
EMPTY_STR, EMPTY_STR, EMPTY_STR,
DB_GETALL_LAND_USE_ZONES, DB_GETBYID_LAND_USE_ZONE) {
}

LandUseZoneDao::~LandUseZoneDao() {
}

void LandUseZoneDao::fromRow(Row& result, LandUseZone& outObj) {
    outObj.id = result.get<BigSerial>(DB_FIELD_ID, INVALID_ID);
    outObj.typeId = result.get<BigSerial>(DB_FIELD_TYPE_ID, INVALID_ID);
    outObj.gpr = result.get<double>(DB_FIELD_GPR, INVALID_DOUBLE);
}

void LandUseZoneDao::toRow(LandUseZone& data, Parameters& outParams, bool update) {
}