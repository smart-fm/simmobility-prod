//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * File:   HouseholdAgent.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on May 16, 2013, 6:36 PM
 */

#include "HouseholdAgent.hpp"
#include "workers/Worker.hpp"
#include "role/impl/HouseholdSellerRole.hpp"
#include "role/impl/HouseholdBidderRole.hpp"

using namespace sim_mob::long_term;
using namespace sim_mob::event;
using namespace sim_mob::messaging;
using sim_mob::Entity;
using std::vector;
using std::string;
using std::map;
using std::endl;

HouseholdAgent::HouseholdAgent(int id, Household* hh, HousingMarket* market)
: LT_Agent(id), market(market), UnitHolder(id), hh(hh) {
    currentRole = new HouseholdSellerRole(this, hh, market);
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

Entity::UpdateStatus HouseholdAgent::OnFrameTick(timeslice now) {
    currentRole->Update(now);
    return Entity::UpdateStatus(UpdateStatus::RS_CONTINUE);
}

void HouseholdAgent::OnFrameOutput(timeslice now) {
}

void HouseholdAgent::HandleMessage(Message::MessageType type, const Message& message) {
    currentRole->HandleMessage(type, message);
}
