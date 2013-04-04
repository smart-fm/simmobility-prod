/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   Seller.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on April 4, 2013, 5:13 PM
 */
#include <iostream>
#include "Seller.hpp"
#include "agent/UnitHolder.hpp"
#include "event/EventPublisher.hpp"

using std::cout;
using std::endl;
using namespace sim_mob;
using namespace sim_mob::long_term;

Seller::Seller(UnitHolder* parent) : Role(parent), cParent(parent) {
    cParent->GetEventManager().RegisterEvent(LTID_BID);
    cParent->GetEventManager().Subscribe(LTID_BID, &UnitHolder::unitX, this,
            CONTEXT_CALLBACK_HANDLER(BidEventArgs, Seller::OnBidReceived));
}

Seller::~Seller() {
}

void Seller::Update(timeslice currTime) {
    if (isActive()) {

    }
}

void Seller::OnBidReceived(EventId id, Context ctx, EventPublisher* sender, const BidEventArgs& args) {
    switch (id) {
        case LTID_BID:// Bid received 
        {
            //take a decision.
            cout << "Id: " << GetParent()->getId() << " Received a bid " << args.GetBid() << endl;
            cParent->GetEventManager().Publish(LTID_BID_RSP, &UnitHolder::unitX, BidEventArgs(ACCEPTED));
            break;
        }
        default:break;
    }
}
