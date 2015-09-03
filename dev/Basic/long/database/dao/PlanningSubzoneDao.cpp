//Copyright (c) 2015 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT)

/*
 * PlanningSubzoneDao.cpp
 *
 *  Created on: 31 Jul, 2015
 *  Author: Chetan Rpgbeer <chetan.rogbeer@smart.mit.edu>
 */

#include <database/dao/PlanningSubzoneDao.hpp>
#include "DatabaseHelper.hpp"

using namespace sim_mob::db;
using namespace sim_mob::long_term;

PlanningSubzoneDao::PlanningSubzoneDao(DB_Connection& connection): SqlAbstractDao<PlanningSubzone>(connection, DB_TABLE_PLANNING_SUBZONE,"", "", "",DB_GETALL_PLANNING_SUBZONE, DB_GETBYID_PLANNING_SUBZONE) {}

PlanningSubzoneDao::~PlanningSubzoneDao() {}

void PlanningSubzoneDao::fromRow(Row& result, PlanningSubzone& outObj)
{
    outObj.id  = result.get<BigSerial>("id", INVALID_ID);
    outObj.planningAreaId  = result.get<BigSerial>("planning_area_id", INVALID_ID);
    outObj.name = result.get<std::string>(	"name", 	EMPTY_STR);
}

void PlanningSubzoneDao::toRow(PlanningSubzone& data, Parameters& outParams, bool update) {}
