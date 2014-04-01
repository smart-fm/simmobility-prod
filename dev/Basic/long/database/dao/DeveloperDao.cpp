//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   DeveloperDao.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on Mar 5, 2013, 5:17 PM
 */

#include "DeveloperDao.hpp"
#include "DatabaseHelper.hpp"

using namespace sim_mob::db;
using namespace sim_mob::long_term;

DeveloperDao::DeveloperDao(DB_Connection& connection)
: SqlAbstractDao<Developer>(connection, DB_TABLE_DEVELOPER,
EMPTY_STR, EMPTY_STR, EMPTY_STR,
DB_GETALL_DEVELOPERS, DB_GETBYID_DEVELOPER) {
}

DeveloperDao::~DeveloperDao() {
}

void DeveloperDao::fromRow(Row& result, Developer& outObj) {
    outObj.id = result.get<BigSerial>(DB_FIELD_ID, INVALID_ID);
    outObj.name = result.get<std::string>(DB_FIELD_NAME, EMPTY_STR);
    outObj.type = result.get<std::string>(DB_FIELD_TYPE, EMPTY_STR);
}

void DeveloperDao::toRow(Developer& data, Parameters& outParams, bool update) {
}