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
#include "database/entity/housing-market/SellerParams.hpp"
#include "database/entity/housing-market/BidderParams.hpp"

using namespace sim_mob::long_term;
using namespace sim_mob::event;
using namespace sim_mob::messaging;
using sim_mob::Entity;
using std::vector;
using std::string;
using std::map;
using std::endl;

HouseholdAgent::HouseholdAgent(int id, Household* hh, const SellerParams& sellerParams,  
        const BidderParams& bidderParams, HousingMarket* market)
: LT_Agent(id), market(market), UnitHolder(id), hh(hh) {
    
    if (id == sellerParams.GetHouseholdId()) {
        currentRole = new HouseholdSellerRole(this, hh, sellerParams, market);
    }
    
    if (id == bidderParams.GetHouseholdId()) {
        currentRole = new HouseholdBidderRole(this, hh, bidderParams, market);
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

Entity::UpdateStatus HouseholdAgent::OnFrameTick(timeslice now) {
    currentRole->Update(now);
    return Entity::UpdateStatus(UpdateStatus::RS_CONTINUE);
}

void HouseholdAgent::OnFrameOutput(timeslice now) {
}

void HouseholdAgent::HandleMessage(Message::MessageType type, const Message& message) {
    currentRole->HandleMessage(type, message);
}