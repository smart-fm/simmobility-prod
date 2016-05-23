/*
 * HouseholdUnit.cpp
 *
 *  Created on: 28 Mar 2016
 *      Author: gishara
 */

#include "HouseholdUnit.hpp"
#include "util/Utils.hpp"

using namespace sim_mob::long_term;

HouseholdUnit::HouseholdUnit(BigSerial houseHoldId,BigSerial unitId,std::tm moveInDate): houseHoldId(houseHoldId), unitId(unitId), moveInDate(moveInDate){}

HouseholdUnit::~HouseholdUnit()
{
}

BigSerial HouseholdUnit::getHouseHoldId() const
{
		return houseHoldId;
}

void HouseholdUnit::setHouseHoldId(BigSerial houseHoldId)
{
		this->houseHoldId = houseHoldId;
}

const std::tm& HouseholdUnit::getMoveInDate() const
{
		return moveInDate;
}

void HouseholdUnit::setMoveInDate(std::tm moveInDate)
{
		this->moveInDate = moveInDate;
}

BigSerial HouseholdUnit::getUnitId() const
{
		return unitId;
}

void HouseholdUnit::setUnitId(BigSerial unitId)
{
		this->unitId = unitId;
}



