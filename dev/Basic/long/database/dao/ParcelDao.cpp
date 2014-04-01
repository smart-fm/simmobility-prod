//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   ParcelDao.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on Mar 10, 2014, 5:17 PM
 */

#include "ParcelDao.hpp"
#include "DatabaseHelper.hpp"

using namespace sim_mob::db;
using namespace sim_mob::long_term;

ParcelDao::ParcelDao(DB_Connection& connection)
: SqlAbstractDao<Parcel>(connection, DB_TABLE_PARCEL,
EMPTY_STR, EMPTY_STR, EMPTY_STR,
DB_GETALL_PARCELS, DB_GETBYID_PARCEL) {
}

ParcelDao::~ParcelDao() {
}

void ParcelDao::fromRow(Row& result, Parcel& outObj) {
    outObj.id = result.get<BigSerial>(DB_FIELD_ID, INVALID_ID);
    outObj.tazId = result.get<BigSerial>(DB_FIELD_TAZ_ID, INVALID_ID);
    outObj.landUseZoneId = result.get<BigSerial>(DB_FIELD_LAND_USE_ZONE_ID, INVALID_ID);
    outObj.area = result.get<double>(DB_FIELD_AREA, .0f);
    outObj.length = result.get<double>(DB_FIELD_LENGTH, .0f);
    outObj.minX = result.get<double>(DB_FIELD_MIN_X, .0f);
    outObj.minY = result.get<double>(DB_FIELD_MIN_Y, .0f);
    outObj.maxX = result.get<double>(DB_FIELD_MAX_X, .0f);
    outObj.maxY = result.get<double>(DB_FIELD_MAX_Y, .0f);
}

void ParcelDao::toRow(Parcel& data, Parameters& outParams, bool update) {
}