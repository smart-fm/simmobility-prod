/*
 * HouseholdPlanningAreaDao.cpp
 *
 *  Created on: 8 Mar 2016
 *      Author: gishara
 */

#include "HouseholdPlanningAreaDao.hpp"
#include "DatabaseHelper.hpp"

using namespace sim_mob::db;
using namespace sim_mob::long_term;

HouseholdPlanningAreaDao::HouseholdPlanningAreaDao(DB_Connection& connection)
: SqlAbstractDao<HouseholdPlanningArea>(connection, EMPTY_STR, EMPTY_STR, EMPTY_STR, EMPTY_STR, DB_GETALL_HH_PLANNING_AREA, EMPTY_STR)
{}

HouseholdPlanningAreaDao::~HouseholdPlanningAreaDao() {
}

void HouseholdPlanningAreaDao::fromRow(Row& result, HouseholdPlanningArea& outObj)
{
    outObj.houseHoldId = result.get<BigSerial>("id", INVALID_ID);
    outObj.tazId = result.get<BigSerial>("taz_id",INVALID_ID);
    outObj.planningArea = result.get<std::string>("planning_area_name",EMPTY_STR);
}

void HouseholdPlanningAreaDao::toRow(HouseholdPlanningArea& data, Parameters& outParams, bool update) {}


