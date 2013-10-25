//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   HouseholdBidderRole.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on May 16, 2013, 5:13 PM
 */

#include <cmath>
#include "HouseholdBidderRole.hpp"
#include "util/UnitHolder.hpp"
#include "message/LT_Message.hpp"
#include "event/EventPublisher.hpp"
#include "event/EventManager.hpp"
#include "agent/impl/HouseholdAgent.hpp"
#include "util/Statistics.hpp"
#include "message/MessageBus.hpp"
#include "model/lua/LuaProvider.hpp"

using std::list;
using std::endl;
using namespace sim_mob::long_term;
using namespace sim_mob::event;
using namespace sim_mob::messaging;

HouseholdBidderRole::HouseholdBidderRole(HouseholdAgent* parent, Household* hh,
        HousingMarket* market)
: LT_AgentRole(parent), market(market), hh(hh), waitingForResponse(false),
lastTime(0, 0), bidOnCurrentDay(false) {
    followMarket();
}

HouseholdBidderRole::~HouseholdBidderRole() {
}

void HouseholdBidderRole::update(timeslice now) {
    //can bid another house if it is not waiting for any 
    //response and if it not the same day
    if (!waitingForResponse && lastTime.ms() < now.ms()) {
        bidOnCurrentDay = false;
    }

    if (isActive()) {
        if (!waitingForResponse && !bidOnCurrentDay && bidUnit(now)) {
            waitingForResponse = true;
            bidOnCurrentDay = true;
        }
    }
    lastTime = now;
}

void HouseholdBidderRole::onWakeUp(EventId id, Context ctx, EventPublisher* sender,
        const EM_EventArgs& args) {
    switch (id) {
        case sim_mob::event::EM_WND_EXPIRED:
        {
            PrintOut("Bidder: [" << getParent()->getId() << "] AWOKE." << endl);
            followMarket();
            setActive(true);
            break;
        }
        default:break;
    }
}

void HouseholdBidderRole::HandleMessage(Message::MessageType type,
        const Message& message) {
    switch (type) {
        case LTMID_BID_RSP:// Bid response received 
        {
            const BidMessage& msg = MSG_CAST(BidMessage, message);
            switch (msg.getResponse()) {
                case ACCEPTED:// Bid accepted 
                {
                    //remove unit from the market.
                    Unit* unit = market->removeUnit(msg.getBid().getUnitId());
                    if (unit) { // assign unit.
                        getParent()->addUnit(unit);
                        setActive(false);
                        PrintOut("Bidder: [" << getParent()->getId() <<
                                "] bid: " << msg.getBid() <<
                                " was accepted " << endl);
                        //sleep for N ticks.
                        timeslice wakeUpTime(lastTime.ms() + 10,
                                lastTime.frame() + 10);
                        /*getParent()->getEventManager().Schedule(wakeUpTime, this,
                                CONTEXT_CALLBACK_HANDLER(EM_EventArgs,
                                HouseholdBidderRole::OnWakeUp));*/
                        unFollowMarket();
                        deleteBidsCounter(unit->getId());
                        Statistics::increment(Statistics::N_ACCEPTED_BIDS);
                    }
                    break;
                }
                case NOT_ACCEPTED:
                {
                    PrintOut("Bidder: [" << getParent()->getId() <<
                            "] bid: " << msg.getBid() <<
                            " was not accepted " << endl);
                    incrementBidsCounter(msg.getBid().getUnitId());
                    break;
                }
                case BETTER_OFFER:
                {
                    deleteBidsCounter(msg.getBid().getUnitId());
                    break;
                }
                case NOT_AVAILABLE:
                {
                    deleteBidsCounter(msg.getBid().getUnitId());
                    break;
                }
                default:break;
            }
            waitingForResponse = false;
            Statistics::increment(Statistics::N_BID_RESPONSES);
            break;
        }
        default:break;
    }
}

void HouseholdBidderRole::onMarketAction(EventId id, EventPublisher* sender,
        const HM_ActionEventArgs& args) {
    switch (id) {
        case LTEID_HM_UNIT_ADDED:// Bid response received 
        {
            //LogOut("Agent: " << getParent()->getId() <<
            //      " notified about the unit added." << endl);
            break;
        }
        case LTEID_HM_UNIT_REMOVED:
        {
            //LogOut("Agent: " << getParent()->getId() <<
            //      " notified about the unit removed." << endl);
            break;
        }
        default:break;
    }
}

bool HouseholdBidderRole::bidUnit(timeslice now) {
    list<Unit*> units;
    market->getUnits(units);
    if (!units.empty()) {
        // choose the unit to bid with max surplus.
        Unit* unit = nullptr;
        float maxSurplus = -1;
        for (list<Unit*>::iterator itr = units.begin(); itr != units.end();
                itr++) {
            if ((*itr)->isAvailable() && ((*itr)->getOwner() != getParent())) {
                float surplus = LuaProvider::getHM_Model().calculateSurplus(*(*itr), getBidsCounter((*(*itr)).getId()));
                if (surplus > maxSurplus) {
                    maxSurplus = surplus;
                    unit = (*itr);
                }
            }
        }
        // Exists some unit to bid.
        if (unit) {
            MessageHandler* owner = dynamic_cast<MessageHandler*> (unit->getOwner());
            float bidValue = maxSurplus + LuaProvider::getHM_Model().calulateWP(*hh, *unit);
            if (owner && bidValue > 0.0f && unit->isAvailable()) {
                //Statistics::Increment(Statistics::N_BIDS);
                MessageBus::PostMessage(owner, LTMID_BID, 
                        MessageBus::MessagePtr(new BidMessage(Bid(unit->getId(), getParent()->getId(),
                        getParent(), bidValue, now))));
                return true;
            }
        }
    }
    return false;
}

void HouseholdBidderRole::followMarket() {
  /*  market->Subscribe(LTEID_HM_UNIT_ADDED, this,
            CALLBACK_HANDLER(HM_ActionEventArgs, HouseholdBidderRole::onMarketAction));
    market->Subscribe(LTEID_HM_UNIT_REMOVED, this,
            CALLBACK_HANDLER(HM_ActionEventArgs, HouseholdBidderRole::onMarketAction));*/
}

void HouseholdBidderRole::unFollowMarket() {
   /* market->UnSubscribe(LTEID_HM_UNIT_ADDED, this);
    market->UnSubscribe(LTEID_HM_UNIT_REMOVED, this);*/
}

int HouseholdBidderRole::getBidsCounter(UnitId unitId) {
    BidsCounterMap::iterator mapItr = bidsPerUnit.find(unitId);
    if (mapItr != bidsPerUnit.end()) {
        return mapItr->second;
    } else {
        bidsPerUnit.insert(BidCounterEntry(unitId, 1));
        return 1;
    }
}

void HouseholdBidderRole::incrementBidsCounter(UnitId unitId) {
    BidsCounterMap::iterator mapItr = bidsPerUnit.find(unitId);
    if (mapItr != bidsPerUnit.end()) {
        (mapItr->second)++;
    } else {
        bidsPerUnit.insert(BidCounterEntry(unitId, 1));
    }
}

void HouseholdBidderRole::deleteBidsCounter(UnitId unitId) {
    bidsPerUnit.erase(unitId);
}
