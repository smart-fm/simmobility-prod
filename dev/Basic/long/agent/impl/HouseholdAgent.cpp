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
#include "model/HM_Model.hpp"
#include "role/LT_Role.hpp"
#include "role/impl/HouseholdBidderRole.hpp"
#include "role/impl/HouseholdSellerRole.hpp"

using namespace sim_mob::long_term;
using namespace sim_mob::event;
using namespace sim_mob::messaging;
using sim_mob::Entity;
using std::vector;
using std::string;
using std::map;
using std::endl;

HouseholdAgent::HouseholdAgent(HM_Model* model, Household* household,
        HousingMarket* market)
: LT_Agent(household->getId()), model(model), market(market), 
        household(household) {
    seller = new HouseholdSellerRole(this);
    bidder = new HouseholdBidderRole(this);
    seller->setActive(true);
    bidder->setActive(true);
}

HouseholdAgent::~HouseholdAgent() {
    safe_delete_item(seller);
    safe_delete_item(bidder);
}

void HouseholdAgent::addUnitId(const BigSerial& unitId) {
    unitIds.push_back(unitId);
}

void HouseholdAgent::removeUnitId(const BigSerial& unitId) {
    unitIds.erase(std::remove(unitIds.begin(), unitIds.end(), unitId),
            unitIds.end());
}

const std::vector<BigSerial>& HouseholdAgent::getUnitIds() const {
    return unitIds;
}

HM_Model* HouseholdAgent::getModel() const{
    return model;
}

HousingMarket* HouseholdAgent::getMarket() const{
    return market;
}

const Household* HouseholdAgent::getHousehold() const{
    return household;
}

bool HouseholdAgent::onFrameInit(timeslice now) {
    return true;
}

Entity::UpdateStatus HouseholdAgent::onFrameTick(timeslice now) {
    if (bidder->isActive()) {
        bidder->update(now);
    }
    
    if (seller->isActive()) {
        seller->update(now);
    }
    return Entity::UpdateStatus(UpdateStatus::RS_CONTINUE);
}

void HouseholdAgent::onFrameOutput(timeslice now) {
}

void HouseholdAgent::OnEvent(EventId eventId,
        EventPublisher* sender, const EventArgs& args) {
}

void HouseholdAgent::OnEvent(EventId eventId, Context ctxId,
        EventPublisher* sender, const EventArgs& args) {

    const HM_ActionEventArgs& hmArgs = MSG_CAST(HM_ActionEventArgs, args);

    switch (eventId) {
        case LTEID_HM_UNIT_ADDED:
        {
            PrintOut("Unit added " << hmArgs.getUnitId() << endl);
            break;
        }
        case LTEID_HM_UNIT_REMOVED:
        {
            PrintOut("Unit removed " << hmArgs.getUnitId() << endl);
            break;
        }
        default:break;
    };
}

void HouseholdAgent::onWorkerEnter() {
    MessageBus::SubscribeEvent(LTEID_HM_UNIT_ADDED, market, this);
    MessageBus::SubscribeEvent(LTEID_HM_UNIT_REMOVED, market, this);
}

void HouseholdAgent::onWorkerExit() {
    MessageBus::UnSubscribeEvent(LTEID_HM_UNIT_ADDED, market, this);
    MessageBus::UnSubscribeEvent(LTEID_HM_UNIT_REMOVED, market, this);
}

void HouseholdAgent::HandleMessage(Message::MessageType type,
        const Message& message) {
    
    if (bidder->isActive()) {
        bidder->HandleMessage(type, message);
    }

    if (seller->isActive()) {
        seller->HandleMessage(type, message);
    }
}
