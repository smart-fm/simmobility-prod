/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   Seller.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on April 4, 2013, 5:13 PM
 */

#include <iostream>
#include "agent/UnitHolder.hpp"
#include "Bidder.hpp"
#include "event/EventPublisher.hpp"

using std::cout;
using std::endl;
using namespace sim_mob;
using namespace sim_mob::long_term;

Bidder::Bidder(UnitHolder* parent) : Role(parent), cParent(parent) {
    cParent->GetEventManager().RegisterEvent(LTID_BID_RSP);
    cParent->GetEventManager().Subscribe(LTID_BID_RSP, &UnitHolder::unitX, this,
            CONTEXT_CALLBACK_HANDLER(BidEventArgs, Bidder::OnBidResponse));
}

Bidder::~Bidder() {
}

void Bidder::Update(timeslice now) {
    if (isActive()) {
        int id = GetParent()->getId();
        cParent->GetEventManager().Publish(LTID_BID, &UnitHolder::unitX, BidEventArgs(id, 10 + id));
        cParent->GetEventManager().Schedule(timeslice(now.ms() + id, now.frame() + id), this,
                CONTEXT_CALLBACK_HANDLER(EM_EventArgs, Bidder::OnWakeUp));
        SetActive(false);
    }
}

void Bidder::OnBidResponse(EventId id, Context ctx, EventPublisher* sender, const BidEventArgs& args) {
    switch (id) {
        case LTID_BID_RSP:// Bid received 
        {
            if (GetParent()->getId() == args.GetBidderId()) {
                cout << "Id: " << GetParent()->getId() << " Received a response " << args.GetResponse() << endl;
            }
            //take a decision.
            break;
        }
        default:break;
    }
}

void Bidder::OnWakeUp(EventId id, Context ctx, EventPublisher* sender, const EM_EventArgs& args) {
    switch (id) {
        case EM_WND_EXPIRED:// Bid received 
        {
            SetActive(true);
            break;
        }
        default:break;
    }
}
