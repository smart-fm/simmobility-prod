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
#include "geospatial/streetdir/StreetDirectory.hpp"
#include "core/DataManager.hpp"

using namespace sim_mob::long_term;
using namespace sim_mob::event;
using namespace sim_mob::messaging;
using sim_mob::Entity;
using std::vector;
using std::string;
using std::map;
using std::endl;

HouseholdAgent::HouseholdAgent(BigSerial id, HM_Model* model, 
        const Household* household, HousingMarket* market, bool marketSeller)
: LT_Agent(id), model(model), market(market), household(household), 
        marketSeller(marketSeller), bidder (nullptr), seller(nullptr) {
    seller = new HouseholdSellerRole(this);
    seller->setActive(marketSeller);
    if (!marketSeller) {
        bidder = new HouseholdBidderRole(this);
        bidder->setActive(false);
    }
}

HouseholdAgent::~HouseholdAgent() {
    safe_delete_item(seller);
    safe_delete_item(bidder);
}

void HouseholdAgent::addUnitId(const BigSerial& unitId) {
    unitIds.push_back(unitId);
    BigSerial tazId = DataManagerSingleton::getInstance().getUnitTazId(unitId);
    if (tazId != INVALID_ID) {
        preferableZones.push_back(tazId);
    }
}

void HouseholdAgent::removeUnitId(const BigSerial& unitId) {
    unitIds.erase(std::remove(unitIds.begin(), unitIds.end(), unitId),
            unitIds.end());
}

const std::vector<BigSerial>& HouseholdAgent::getUnitIds() const {
    return unitIds;
}

const std::vector<BigSerial>& HouseholdAgent::getPreferableZones() const {
    return preferableZones;
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
    if (bidder && bidder->isActive()) {
        bidder->update(now);
    }
    
    if (seller && seller->isActive()) {
        seller->update(now);
    }
    return Entity::UpdateStatus(UpdateStatus::RS_CONTINUE);
}

void HouseholdAgent::onFrameOutput(timeslice now) {
}

void HouseholdAgent::onEvent(EventId eventId, Context ctxId,
        EventPublisher*, const EventArgs& args) {
        processEvent(eventId, ctxId, args);
}

void HouseholdAgent::processEvent(EventId eventId, Context ctxId,
        const EventArgs& args) {
    switch (eventId) {
        case LTEID_HM_UNIT_ADDED:
        {
            const HM_ActionEventArgs& hmArgs = MSG_CAST(HM_ActionEventArgs, args);
            PrintOut("Unit added " << hmArgs.getUnitId() << endl);
            break;
        }
        case LTEID_HM_UNIT_REMOVED:
        {
            const HM_ActionEventArgs& hmArgs = MSG_CAST(HM_ActionEventArgs, args);
            PrintOut("Unit removed " << hmArgs.getUnitId() << endl);
            break;
        }
        case LTEID_EXT_LOST_JOB:
        case LTEID_EXT_NEW_CHILD:
        case LTEID_EXT_NEW_JOB:
        case LTEID_EXT_NEW_JOB_LOCATION:
        case LTEID_EXT_NEW_SCHOOL_LOCATION:  
        {
            const ExternalEventArgs& exArgs = MSG_CAST(ExternalEventArgs, args);
            if (exArgs.getEvent().getHouseholdId() == getId()) {
                processExternalEvent(exArgs);
            }
            break;
        }
        default:break;
    };
}

void HouseholdAgent::processExternalEvent(const ExternalEventArgs& args) {
    switch(args.getEvent().getType()){
        case ExternalEvent::LOST_JOB:
        case ExternalEvent::NEW_CHILD:
        case ExternalEvent::NEW_JOB:
        case ExternalEvent::NEW_JOB_LOCATION:
        case ExternalEvent::NEW_SCHOOL_LOCATION:
        {
            if (seller){
                seller->setActive(true);
            }
            if (bidder){
                bidder->setActive(true);
            }
            break;
        }
        default:break;
    }
}


void HouseholdAgent::onWorkerEnter() {
    if (!marketSeller) {
        MessageBus::SubscribeEvent(LTEID_EXT_NEW_JOB, this, this);
        MessageBus::SubscribeEvent(LTEID_EXT_NEW_CHILD, this, this);
        MessageBus::SubscribeEvent(LTEID_EXT_LOST_JOB, this, this);
        MessageBus::SubscribeEvent(LTEID_EXT_NEW_SCHOOL_LOCATION, this, this);
        MessageBus::SubscribeEvent(LTEID_EXT_NEW_JOB_LOCATION, this, this);
        //MessageBus::SubscribeEvent(LTEID_HM_UNIT_ADDED, market, this);
        //MessageBus::SubscribeEvent(LTEID_HM_UNIT_REMOVED, market, this);
    }
}

void HouseholdAgent::onWorkerExit() {
    if (!marketSeller) {
        MessageBus::UnSubscribeEvent(LTEID_EXT_NEW_JOB, this, this);
        MessageBus::UnSubscribeEvent(LTEID_EXT_NEW_CHILD, this, this);
        MessageBus::UnSubscribeEvent(LTEID_EXT_LOST_JOB, this, this);
        MessageBus::UnSubscribeEvent(LTEID_EXT_NEW_SCHOOL_LOCATION, this, this);
        MessageBus::UnSubscribeEvent(LTEID_EXT_NEW_JOB_LOCATION, this, this);
        //MessageBus::UnSubscribeEvent(LTEID_HM_UNIT_ADDED, market, this);
        //MessageBus::UnSubscribeEvent(LTEID_HM_UNIT_REMOVED, market, this);
    }
}

void HouseholdAgent::HandleMessage(Message::MessageType type,
        const Message& message) {
    
    if (bidder && bidder->isActive()) {
        bidder->HandleMessage(type, message);
    }

    if (seller && seller->isActive()) {
        seller->HandleMessage(type, message);
    }
}
