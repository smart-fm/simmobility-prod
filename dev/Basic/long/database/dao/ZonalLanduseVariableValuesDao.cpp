//Copyright (c) 2015 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT)

/*
 * ZonalLanduseVariableValuesDao.cpp
 *
 *  Created on: 11 Aug, 2015
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#include <database/dao/ZonalLanduseVariableValuesDao.hpp>
#include "DatabaseHelper.hpp"

using namespace sim_mob::db;
using namespace sim_mob::long_term;

ZonalLanduseVariableValuesDao::ZonalLanduseVariableValuesDao(DB_Connection& connection): SqlAbstractDao<ZonalLanduseVariableValues>(connection, DB_TABLE_ZONALLANDUSEVARIABLEVALUES, EMPTY_STR, EMPTY_STR, EMPTY_STR, DB_GETALL_ZONALLANDUSEVARIABLEVALUES, DB_GETBYID_ZONALLANDUSEVARIABLEVALUES) {}

ZonalLanduseVariableValuesDao::~ZonalLanduseVariableValuesDao() {}

void ZonalLanduseVariableValuesDao::fromRow(Row& result, ZonalLanduseVariableValues& outObj)
{
    outObj.alt_id  		= result.get<int>("alt_id", INVALID_ID);
    outObj.dgpid 		= result.get<int>("dgpid", INVALID_ID);
    outObj.dwl 			= result.get<int>("dwl", INVALID_ID);
    outObj.f_loc_com	= result.get<double>("f_loc_com", 0.0);
    outObj.f_loc_res	= result.get<double>("f_loc_res", 0.0);
    outObj.f_loc_open	= result.get<double>("f_loc_open", 0.0);
    outObj.odi10_loc	= result.get<double>("odi10_loc", 0.0);
    outObj.dis2mrt		= result.get<double>("dis2mrt", 0.0);
    outObj.dis2exp		= result.get<double>("dis2exp", 0.0);

}

void ZonalLanduseVariableValuesDao::toRow(ZonalLanduseVariableValues& data, Parameters& outParams, bool update) {}





