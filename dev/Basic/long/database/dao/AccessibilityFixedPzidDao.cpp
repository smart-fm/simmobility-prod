//Copyright (c) 2016 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT)

/*
 * AccessibilityFixedPzidDao.cpp
 *
 *  Created on: 20 Jan 2016
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#include <database/dao/AccessibilityFixedPzidDao.hpp>
#include "DatabaseHelper.hpp"

using namespace sim_mob::db;
using namespace sim_mob::long_term;

AccessibilityFixedPzidDao::AccessibilityFixedPzidDao(DB_Connection& connection): SqlAbstractDao<AccessibilityFixedPzid>( connection, DB_TABLE_ACCESSIBILITYFIXEDPZID,
																				 "", "", "", DB_GETALL_ACCESSIBILITYFIXEDPZID, DB_GETBYID_ACCESSIBILITYFIXEDPZID){}

AccessibilityFixedPzidDao::~AccessibilityFixedPzidDao(){}

void AccessibilityFixedPzidDao::fromRow(Row& result, AccessibilityFixedPzid& outObj)
{
	outObj.id				= result.get<BigSerial>( "id", INVALID_ID);
    outObj.planningAreaId	= result.get<int>( "planning_area_id", INVALID_ID);
    outObj.dgp 				= result.get<std::string>( "DGP", "");
    outObj.accTMfg			= result.get<double>( "acc_t_mfg", 0);
    outObj.accTOff			= result.get<double>( "acc_t_off", 0);
}

void AccessibilityFixedPzidDao::toRow(AccessibilityFixedPzid& data, Parameters& outParams, bool update) {}


