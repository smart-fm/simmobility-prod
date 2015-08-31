//Copyright (c) 2015 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT)

/*
 * PlanningAreaDao.cpp
 *
 * Created on: July 30, 2015
 * Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#include <database/dao/PlanningAreaDao.hpp>
#include "DatabaseHelper.hpp"

using namespace sim_mob::db;
using namespace sim_mob::long_term;

PlanningAreaDao::PlanningAreaDao(DB_Connection& connection): SqlAbstractDao<PlanningArea>(connection, DB_TABLE_PLANNING_AREA,"", "", "",DB_GETALL_PLANNING_AREA, DB_GETBYID_PLANNING_AREA)
{}

PlanningAreaDao::~PlanningAreaDao() {}

void PlanningAreaDao::fromRow(Row& result, PlanningArea& outObj)
{
    outObj.id 	= result.get<BigSerial>(	"id", 		INVALID_ID);
    outObj.name = result.get<std::string>(	"name", 	EMPTY_STR);
}

void PlanningAreaDao::toRow(PlanningArea& data, Parameters& outParams, bool update) {}
