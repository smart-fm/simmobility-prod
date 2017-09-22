/*
 * HouseholdUnitDao.cpp
 *
 *  Created on: 28 Mar 2016
 *      Author: gishara
 */

#include "HouseholdUnitDao.hpp"
#include "DatabaseHelper.hpp"

using namespace sim_mob::db;
using namespace sim_mob::long_term;

HouseholdUnitDao::HouseholdUnitDao(DB_Connection& connection): SqlAbstractDao<HouseholdUnit>(connection, EMPTY_STR,EMPTY_STR, EMPTY_STR, EMPTY_STR,EMPTY_STR, EMPTY_STR)
{}

HouseholdUnitDao::~HouseholdUnitDao() {}

void HouseholdUnitDao::fromRow(Row& result, HouseholdUnit& outObj)
{
    outObj.houseHoldId = result.get<BigSerial>("household_id", INVALID_ID);
    outObj.unitId = result.get<BigSerial>("unit_id", INVALID_ID);
    outObj.moveInDate = result.get<std::tm>("move_in_date", std::tm());
}

void HouseholdUnitDao::toRow(HouseholdUnit& data, Parameters& outParams, bool update)
{
	outParams.push_back(data.getHouseHoldId());
	outParams.push_back(data.getUnitId());
	outParams.push_back(data.getMoveInDate());
}

void HouseholdUnitDao::insertHouseholdUnit(HouseholdUnit& houseHold,std::string schema)
{
	const std::string DB_INSERT_HOUSEHOLD_UNIT = "INSERT INTO " + schema + ".household_unit"
		+ " (" + "household_id" + ", "+ "unit_id" + ", "+ "move_in_date" + ") VALUES (:v1, :v2, :v3)";
	insertViaQuery(houseHold,DB_INSERT_HOUSEHOLD_UNIT);
}




