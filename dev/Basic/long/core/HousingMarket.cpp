//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   HousingMarket.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on March 11, 2013, 4:13 PM
 */

#include "HousingMarket.hpp"
#include "workers/Worker.hpp"
#include "event/LT_EventArgs.hpp"
#include "message/MessageBus.hpp"

using namespace sim_mob::long_term;
using namespace sim_mob::event;
using namespace sim_mob::messaging;
using sim_mob::Entity;
using std::vector;

UnitEntry::UnitEntry(Unit& unit, double price)
: unit(unit), price(price) {
}

UnitEntry::UnitEntry(const UnitEntry& orig)
: unit(orig.unit), price(orig.price) {
}

UnitEntry::~UnitEntry() {
}

UnitEntry& UnitEntry::operator=(const UnitEntry& source) {
    this->unit = source.unit;
    this->price = source.price;
}

const Unit& UnitEntry::getUnit() const {
    return unit;
}

double UnitEntry::getPrice() const {
    return price;
}

HousingMarket::HousingMarket() : UnitHolder(-1), Entity(-1) {
}

HousingMarket::~HousingMarket() {
}

bool HousingMarket::add(Unit* unit, bool reOwner) {
    // no re-parent the unit to the market.
    bool retVal = UnitHolder::add(unit, false);
    if (retVal) {
        MessageBus::PublishEvent(LTEID_HM_UNIT_ADDED, this,
                MessageBus::EventArgsPtr(new HM_ActionEventArgs(unit->getId())));
    }
    return retVal;
}

Unit* HousingMarket::remove(UnitId id, bool reOwner) {
    // no re-parent the unit to the market.
    Unit* retVal = UnitHolder::remove(id, false);
    if (retVal) {
        MessageBus::PublishEvent(LTEID_HM_UNIT_REMOVED, this,
                MessageBus::EventArgsPtr(new HM_ActionEventArgs(id)));
    }
    return retVal;
}

Entity::UpdateStatus HousingMarket::update(timeslice now) {
    return Entity::UpdateStatus(Entity::UpdateStatus::RS_CONTINUE);
}

bool HousingMarket::isNonspatial() {
    return false;
}

void HousingMarket::buildSubscriptionList(vector<BufferedBase*>& subsList) {
}

void HousingMarket::onWorkerEnter() {
}

void HousingMarket::onWorkerExit() {
}
