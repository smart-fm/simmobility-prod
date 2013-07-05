/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   HousingMarket.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on March 11, 2013, 4:13 PM
 */

#include "HousingMarket.hpp"
#include "workers/Worker.hpp"
#include "event/LT_EventArgs.hpp"

using namespace sim_mob::long_term;
using sim_mob::Entity;
using sim_mob::EventPublisher;
using std::vector;

HousingMarket::HousingMarket() : UnitHolder(-1), Entity(-1), firstTime(true) {
    RegisterEvent(LTEID_HM_UNIT_ADDED);
    RegisterEvent(LTEID_HM_UNIT_REMOVED);
}

HousingMarket::~HousingMarket() {
    UnRegisterEvent(LTEID_HM_UNIT_ADDED);
    UnRegisterEvent(LTEID_HM_UNIT_REMOVED);
}

bool HousingMarket::Add(Unit* unit, bool reOwner) {
    // no re-parent the unit to the market.
    bool retVal = UnitHolder::Add(unit, false);
    if (retVal) {
        Publish(LTEID_HM_UNIT_ADDED, HM_ActionEventArgs(unit->GetId()));
    }
    return retVal;
}

Unit* HousingMarket::Remove(UnitId id, bool reOwner) {
    // no re-parent the unit to the market.
    Unit* retVal = UnitHolder::Remove(id, false);
    if (retVal) {
        Publish(LTEID_HM_UNIT_REMOVED, HM_ActionEventArgs(id));
    }
    return retVal;
}

Entity::UpdateStatus HousingMarket::update(timeslice now) {
    Setup();
    return Entity::UpdateStatus(Entity::UpdateStatus::RS_CONTINUE);
}

bool HousingMarket::isNonspatial() {
    return false;
}

void HousingMarket::buildSubscriptionList(vector<BufferedBase*>& subsList) {
}

void HousingMarket::Setup() {
    if (firstTime) {
        //setup first things inside the entity.
        firstTime = false;
    }
}