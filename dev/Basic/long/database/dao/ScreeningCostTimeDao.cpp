//Copyright (c) 2016 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT)


/*
 * ScreeningCostTimeDao.cpp
 *
 *  Created on: 20 Jan 2016
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#include <database/dao/ScreeningCostTimeDao.hpp>
#include "DatabaseHelper.hpp"

using namespace sim_mob::db;
using namespace sim_mob::long_term;

ScreeningCostTimeDao::ScreeningCostTimeDao(DB_Connection& connection): SqlAbstractDao<ScreeningCostTime>( connection, DB_TABLE_SCREENINGCOSTTIME,
																			 "", "", "", DB_GETALL_SCREENINGCOSTTIME, DB_GETBYID_SCREENINGCOSTTIME){}

ScreeningCostTimeDao::~ScreeningCostTimeDao(){}

void ScreeningCostTimeDao::fromRow(Row& result, ScreeningCostTime& outObj)
{
	outObj.id						= result.get<BigSerial>( "id", INVALID_ID);
    outObj.planningAreaOrigin		= result.get<int>( "planning_area_origin", INVALID_ID);
    outObj.areaOrigin 				= result.get<std::string>( "area_origin", "");
    outObj.planningAreaDestination 	= result.get<int>( "planning_area_destination", INVALID_ID);
    outObj.areaDestination			= result.get<std::string>( "area_destination", "");
    outObj.time						= result.get<double>( "time", 0);
    outObj.cost						= result.get<double>( "cost", 0);
}

void ScreeningCostTimeDao::toRow(ScreeningCostTime& data, Parameters& outParams, bool update) {}


