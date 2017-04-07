//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * TazDao.cpp
 *
 *  Created on: 23 Jun, 2015
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#include <database/dao/TazDao.hpp>
#include "DatabaseHelper.hpp"

using namespace sim_mob::db;
using namespace sim_mob::long_term;

TazDao::TazDao(DB_Connection& connection): SqlAbstractDao<Taz>(connection, "","", "", "","SELECT * FROM " + connection.getSchema()+"taz", ""){}

TazDao::~TazDao() {}

void TazDao::fromRow(Row& result, Taz& outObj)
{
    outObj.id = result.get<BigSerial>(DB_FIELD_ID, INVALID_ID);
    outObj.name = result.get<std::string>(DB_FIELD_NAME, "");
    outObj.area = result.get<double>("area", 0.0);
    outObj.surcharge = result.get<int>("surcharge", 0);
    outObj.status_0812 = result.get<int>("status_0812", 0);
    outObj.mtzName = result.get<std::string>("mtz_name", "");
    outObj.subzoneName = result.get<std::string>("subzone_name", "");
    outObj.planningAreaName = result.get<std::string>("planning_area_name", "");
}

void TazDao::toRow(Taz& data, Parameters& outParams, bool update) {}
