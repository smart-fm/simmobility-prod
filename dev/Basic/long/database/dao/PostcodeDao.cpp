//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   PostcodeDao.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on Feb 11, 2014, 3:59 PM
 */

#include "PostcodeDao.hpp"
#include "DatabaseHelper.hpp"

using namespace sim_mob::db;
using namespace sim_mob::long_term;

PostcodeDao::PostcodeDao(DB_Connection& connection)
: SqlAbstractDao<Postcode>(connection, DB_TABLE_POSTCODE,
DB_INSERT_POSTCODE, DB_UPDATE_POSTCODE, DB_DELETE_POSTCODE,
DB_GETALL_POSTCODE, DB_GETBYID_POSTCODE) {}

PostcodeDao::~PostcodeDao() {
}

void PostcodeDao::fromRow(Row& result, Postcode& outObj) {
    outObj.id = result.get<BigSerial>(DB_FIELD_ID, INVALID_ID);
    outObj.tazId = result.get<BigSerial>(DB_FIELD_TAZ_ID, INVALID_ID);
    outObj.code = result.get<std::string>(DB_FIELD_CODE, EMPTY_STR);
    outObj.location.latitude = result.get<double>(DB_FIELD_LATITUDE, 0);
    outObj.location.longitude = result.get<double>(DB_FIELD_LONGITUDE, 0);
}

void PostcodeDao::toRow(Postcode& data, Parameters& outParams, bool update) {
}
