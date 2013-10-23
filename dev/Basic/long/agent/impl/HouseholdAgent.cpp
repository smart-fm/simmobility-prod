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
#include "message/MessageBus.hpp"
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
    bidderRole = new HouseholdBidderRole(this, hh, market);
    sellerRole = new HouseholdSellerRole(this, hh, market);
    sellerRole->SetActive(true);
}

HouseholdAgent::~HouseholdAgent() {
    safe_delete_item(bidderRole);
    safe_delete_item(sellerRole);
}

bool HouseholdAgent::onFrameInit(timeslice now) {
    bool retVal = (hh && market);
    if (!retVal) {
        LogOut("HouseholdAgent::OnFrameInit - Some information is invalid." << endl);
    }
    return retVal;
}

Entity::UpdateStatus HouseholdAgent::onFrameTick(timeslice now) {
    if (bidderRole && bidderRole->isActive()) {
        bidderRole->Update(now);
    }
    if (sellerRole && sellerRole->isActive()) {
        sellerRole->Update(now);
    }
    return Entity::UpdateStatus(UpdateStatus::RS_CONTINUE);
}

void HouseholdAgent::onFrameOutput(timeslice now) {
}

void HouseholdAgent::OnEvent(EventId eventId, EventPublisher* sender, const EventArgs& args) {
}

void HouseholdAgent::OnEvent(EventId eventId, Context ctxId, EventPublisher* sender, const EventArgs& args) {

    switch (eventId) {
        case LTEID_HM_UNIT_ADDED:
        {
            const HM_ActionEventArgs& hmArgs = static_cast<const HM_ActionEventArgs&> (args);
            PrintOut("Unit added " << hmArgs.GetUnitId() << endl);
            bidderRole->SetActive(true);
            break;
        }
        case LTEID_HM_UNIT_REMOVED:
        {
            const HM_ActionEventArgs& hmArgs = static_cast<const HM_ActionEventArgs&> (args);
            PrintOut("Unit removed " << hmArgs.GetUnitId() << endl);
            break;
        }
        default:break;
    };
}

void HouseholdAgent::HandleMessage(Message::MessageType type, const Message& message) {
    if (bidderRole && bidderRole->isActive()) {
        bidderRole->HandleMessage(type, message);
    }
    if (sellerRole && sellerRole->isActive()) {
        sellerRole->HandleMessage(type, message);
    }
}

void HouseholdAgent::onWorkerEnter() {
    MessageBus::SubscribeEvent(LTEID_HM_UNIT_ADDED, market, this);
    MessageBus::SubscribeEvent(LTEID_HM_UNIT_REMOVED, market, this);
}

void HouseholdAgent::onWorkerExit() {
    MessageBus::UnSubscribeEvent(LTEID_HM_UNIT_ADDED, market, this);
    MessageBus::UnSubscribeEvent(LTEID_HM_UNIT_REMOVED, market, this);
}
            