/*
 * HouseholdPlanningArea.cpp
 *
 *  Created on: 8 Mar 2016
 *      Author: gishara
 */
#include "HouseholdPlanningArea.hpp"
#include "util/Utils.hpp"

using namespace sim_mob::long_term;

HouseholdPlanningArea::HouseholdPlanningArea(BigSerial houseHoldId, BigSerial tazId, std::string planningArea): houseHoldId(houseHoldId), tazId(tazId),planningArea(planningArea){}

HouseholdPlanningArea::~HouseholdPlanningArea() {
}

BigSerial HouseholdPlanningArea::getHouseHoldId() const {
	return houseHoldId;
}

void HouseholdPlanningArea::setHouseHoldId(BigSerial houseHoldId) {
	this->houseHoldId = houseHoldId;
}

const std::string& HouseholdPlanningArea::getPlanningArea() const {
	return planningArea;
}

void HouseholdPlanningArea::setPlanningArea(const std::string& planningArea) {
	this->planningArea = planningArea;
}

BigSerial HouseholdPlanningArea::getTazId() const {
	return tazId;
}

void HouseholdPlanningArea::setTazId(BigSerial tazId) {
	this->tazId = tazId;
}



