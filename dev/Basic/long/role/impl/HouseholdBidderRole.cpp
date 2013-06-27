/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
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
#include "agent/impl/HouseholdAgent.hpp"
#include "util/Statistics.hpp"

using std::list;
using std::endl;
using namespace sim_mob::long_term;

HouseholdBidderRole::HouseholdBidderRole(HouseholdAgent* parent, Household* hh,
        HousingMarket* market)
: LT_AgentRole(parent), market(market), hh(hh), waitingForResponse(false),
lastTime(0, 0), bidOnCurrentDay(false) {
    FollowMarket();
}

HouseholdBidderRole::~HouseholdBidderRole() {
}

void HouseholdBidderRole::Update(timeslice now) {
    //can bid another house if it is not waiting for any 
    //response and if it not the same day
    if (!waitingForResponse && lastTime.ms() < now.ms()) {
        bidOnCurrentDay = false;
    }

    if (isActive()) {
        if (!waitingForResponse && !bidOnCurrentDay && BidUnit(now)) {
            waitingForResponse = true;
            bidOnCurrentDay = true;
        }
    }
    lastTime = now;
}

void HouseholdBidderRole::OnWakeUp(EventId id, Context ctx, EventPublisher* sender,
        const EM_EventArgs& args) {
    switch (id) {
        case EM_WND_EXPIRED:
        {
            LogOut("Bidder: [" << GetParent()->getId() << "] AWOKE." << endl);
            FollowMarket();
            SetActive(true);
            break;
        }
        default:break;
    }
}

void HouseholdBidderRole::HandleMessage(MessageType type, MessageReceiver& sender,
        const Message& message) {
    switch (type) {
        case LTMID_BID_RSP:// Bid response received 
        {
            BidMessage* msg = MSG_CAST(BidMessage, message);
            switch (msg->GetResponse()) {
                case ACCEPTED:// Bid accepted 
                {
                    //remove unit from the market.
                    Unit* unit = market->RemoveUnit(msg->GetBid().GetUnitId());
                    if (unit) { // assign unit.
                        GetParent()->AddUnit(unit);
                        SetActive(false);
                        LogOut("Bidder: [" << GetParent()->getId() <<
                                "] bid: " << msg->GetBid() <<
                                " was accepted " << endl);
                        //sleep for N ticks.
                        timeslice wakeUpTime(lastTime.ms() + 10,
                                lastTime.frame() + 10);
                        GetParent()->GetEventManager().Schedule(wakeUpTime, this,
                                CONTEXT_CALLBACK_HANDLER(EM_EventArgs,
                                HouseholdBidderRole::OnWakeUp));
                        UnFollowMarket();
                        DeleteBidsCounter(unit->GetId());
                        Statistics::Increment(Statistics::N_ACCEPTED_BIDS);
                    }
                    break;
                }
                case NOT_ACCEPTED:
                {
                    LogOut("Bidder: [" << GetParent()->getId() <<
                            "] bid: " << msg->GetBid() <<
                            " was not accepted " << endl);
                    IncrementBidsCounter(msg->GetBid().GetUnitId());
                    break;
                }
                case BETTER_OFFER:
                {
                    DeleteBidsCounter(msg->GetBid().GetUnitId());
                    break;
                }
                case NOT_AVAILABLE:
                {
                    DeleteBidsCounter(msg->GetBid().GetUnitId());
                    break;
                }
                default:break;
            }
            waitingForResponse = false;
            Statistics::Increment(Statistics::N_BID_RESPONSES);
            break;
        }
        default:break;
    }
}

void HouseholdBidderRole::OnMarketAction(EventId id, EventPublisher* sender,
        const HM_ActionEventArgs& args) {
    switch (id) {
        case LTEID_HM_UNIT_ADDED:// Bid response received 
        {
            //LogOut("Agent: " << GetParent()->getId() <<
            //      " notified about the unit added." << endl);
            break;
        }
        case LTEID_HM_UNIT_REMOVED:
        {
            //LogOut("Agent: " << GetParent()->getId() <<
            //      " notified about the unit removed." << endl);
            break;
        }
        default:break;
    }
}

bool HouseholdBidderRole::BidUnit(timeslice now) {
    list<Unit*> units;
    market->GetUnits(units);
    if (!units.empty()) {
        // choose the unit to bid with max surplus.
        Unit* unit = nullptr;
        float maxSurplus = -1;
        for (list<Unit*>::iterator itr = units.begin(); itr != units.end();
                itr++) {
            if ((*itr)->IsAvailable()) {
                float surplus = CalculateSurplus(*(*itr));
                if (surplus > maxSurplus) {
                    maxSurplus = surplus;
                    unit = (*itr);
                }
            }
        }
        // Exists some unit to bid.
        if (unit) {
            MessageReceiver* owner = dynamic_cast<MessageReceiver*> (unit->GetOwner());
            float bidValue = maxSurplus + CalculateWP(*unit);
            if (owner && bidValue > 0.0f && unit->IsAvailable()) {
                //Statistics::Increment(Statistics::N_BIDS);
                owner->Post(LTMID_BID, GetParent(),
                        new BidMessage(Bid(unit->GetId(), GetParent()->getId(),
                        GetParent(), bidValue, now)));
                return true;
            }
        }
    }
    return false;
}

float HouseholdBidderRole::CalculateSurplus(const Unit& unit) {
    /*float askingPrice = unit.GetHedonicPrice(); //needs to be reviewed by Victor.
    return pow(askingPrice, hh->GetWeightUrgencyToBuy() + 1) /
            ((float) GetBidsCounter(unit.GetId()) * unit.GetWeightPriceQuality());*/
    return 0;
}

float HouseholdBidderRole::CalculateWP(const Unit& unit) {
    return 0;/*(float) ((hh->GetWeightIncome() * hh->GetIncome()) +
            (hh->GetWeightDistanceToCDB() * unit.GetDistanceToCDB()) +
            (hh->GetWeightUnitSize() * unit.GetSize()));*/
}

void HouseholdBidderRole::FollowMarket() {
    market->Subscribe(LTEID_HM_UNIT_ADDED, this,
            CALLBACK_HANDLER(HM_ActionEventArgs, HouseholdBidderRole::OnMarketAction));
    market->Subscribe(LTEID_HM_UNIT_REMOVED, this,
            CALLBACK_HANDLER(HM_ActionEventArgs, HouseholdBidderRole::OnMarketAction));
}

void HouseholdBidderRole::UnFollowMarket() {
    market->UnSubscribe(LTEID_HM_UNIT_ADDED, this);
    market->UnSubscribe(LTEID_HM_UNIT_REMOVED, this);
}

int HouseholdBidderRole::GetBidsCounter(UnitId unitId) {
    BidsCounterMap::iterator mapItr = bidsPerUnit.find(unitId);
    if (mapItr != bidsPerUnit.end()) {
        return mapItr->second;
    } else {
        bidsPerUnit.insert(BidCounterEntry(unitId, 1));
        return 1;
    }
}

void HouseholdBidderRole::IncrementBidsCounter(UnitId unitId) {
    BidsCounterMap::iterator mapItr = bidsPerUnit.find(unitId);
    if (mapItr != bidsPerUnit.end()) {
        (mapItr->second)++ ;
    } else {
        bidsPerUnit.insert(BidCounterEntry(unitId, 1));
    }
}

void HouseholdBidderRole::DeleteBidsCounter(UnitId unitId) {
    bidsPerUnit.erase(unitId);
}