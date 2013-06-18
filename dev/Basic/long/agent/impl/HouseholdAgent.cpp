/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   HouseholdAgent.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on May 16, 2013, 6:36 PM
 */

#include "HouseholdAgent.hpp"
#include "conf/simpleconf.hpp"
#include "workers/Worker.hpp"
#include "role/impl/HouseholdSellerRole.hpp"
#include "role/impl/HouseholdBidderRole.hpp"

using namespace sim_mob;
using namespace long_term;

HouseholdAgent::HouseholdAgent(int id, Household* hh, HousingMarket* market)
: LT_Agent(id), market(market), UnitHolder(id), hh(hh) {
    if (id % 2 == 0) {
        currentRole = new HouseholdSellerRole(this, hh, market);
    } else {
        currentRole = new HouseholdBidderRole(this, hh, market);
    }
    currentRole->SetActive(true);
}

HouseholdAgent::~HouseholdAgent() {
}

bool HouseholdAgent::OnFrameInit(timeslice now) {
    bool retVal = (hh && market);
    if (!retVal) {
        LogOut("HouseholdAgent::OnFrameInit - Some information is invalid." << endl);
    }
    return retVal;
}

Entity::UpdateStatus HouseholdAgent::OnFrameTick(timeslice now, int messageCounter) {
    currentRole->Update(now);
    return Entity::UpdateStatus(UpdateStatus::RS_CONTINUE);
}

void HouseholdAgent::OnFrameOutput(timeslice now) {
}

void HouseholdAgent::HandleMessage(MessageType type, MessageReceiver& sender,
        const Message& message) {
    currentRole->HandleMessage(type, sender, message);
}