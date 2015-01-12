//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   LT_EventArgs.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on April 4, 2013, 5:42 PM
 */

#include "LT_EventArgs.hpp"

using namespace sim_mob::long_term;
using sim_mob::event::EventArgs;

HM_ActionEventArgs::HM_ActionEventArgs(BigSerial unitId,BigSerial buildingId, std::tm futureDemolitionDate, Unit *unit)
: unitId(unitId),buildingId(buildingId),buildingFutureDemolitionDate(buildingFutureDemolitionDate) ,unit(unit){
}

HM_ActionEventArgs::HM_ActionEventArgs(const HM_ActionEventArgs& source)
: unitId(source.unitId),buildingId(source.buildingId), buildingFutureDemolitionDate(source.buildingFutureDemolitionDate){
}

HM_ActionEventArgs::HM_ActionEventArgs(Unit &unit)
: unitId(unit.getId()),buildingId(unit.getBuildingId()), buildingFutureDemolitionDate(std::tm()), unit(&unit){
}

HM_ActionEventArgs::~HM_ActionEventArgs() {
}

BigSerial HM_ActionEventArgs::getUnitId() const {
    return unitId;
}

BigSerial HM_ActionEventArgs::getBuildingId() const {
    return buildingId;
}

std::tm HM_ActionEventArgs::getFutureDemolitionDate() const {
	return buildingFutureDemolitionDate;
}

Unit *HM_ActionEventArgs::getUnit() const
{
	return unit;
}
/******************************************************************************
 *                              ExternalEventArgs   
 ******************************************************************************/
ExternalEventArgs::ExternalEventArgs(const ExternalEvent& event)
: event(event) {

}

ExternalEventArgs::ExternalEventArgs(const ExternalEventArgs& orig)
: event(orig.event) {
}

ExternalEventArgs::~ExternalEventArgs() {
}

const ExternalEvent& ExternalEventArgs::getEvent() const {
    return event;
}
