/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   HouseholdBidderRole.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on May 16, 2013, 5:13 PM
 */

#include <math.h>
#include "HouseholdBidderRole.hpp"
#include "model/UnitHolder.hpp"
#include "message/LT_Message.hpp"
#include "event/EventPublisher.hpp"
#include "agent/impl/HouseholdAgent.hpp"


using namespace sim_mob;
using namespace sim_mob::long_term;

HouseholdBidderRole::HouseholdBidderRole(HouseholdAgent* parent, Household* hh,
        HousingMarket* market)
: LT_AgentRole(parent), market(market), hh(hh), waitingForResponse(false),
lastTime(0, 0) {
    FollowMarket();
}

HouseholdBidderRole::~HouseholdBidderRole() {
}

void HouseholdBidderRole::Update(timeslice now) {
    if (isActive()) {
        if (!waitingForResponse && BidUnit()) {
            waitingForResponse = true;
        }
    }
    lastTime = now;
}

void HouseholdBidderRole::OnWakeUp(EventId id, Context ctx, EventPublisher* sender,
        const EM_EventArgs& args) {
    switch (id) {
        case EM_WND_EXPIRED:
        {
            LogOut("Agent: " << GetParent()->getId() << " AWOKE." << endl);
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
                        LogOut("Agent: " << GetParent()->getId() <<
                                " bought the unit: " << msg->GetBid().GetUnitId()
                                << endl);
                        //sleep for N ticks.
                        timeslice wakeUpTime(lastTime.ms() + 10,
                                lastTime.frame() + 10);
                        GetParent()->GetEventManager().Schedule(wakeUpTime, this,
                                CONTEXT_CALLBACK_HANDLER(EM_EventArgs,
                                HouseholdBidderRole::OnWakeUp));
                        UnFollowMarket();
                    }
                    break;
                }
                case NOT_ACCEPTED:
                {
                    LogOut("Agent: " << GetParent()->getId() <<
                            " bid to unit: " << msg->GetBid().GetUnitId() <<
                            " was not accepted." << endl);
                    break;
                }
                default:break;
            }
            waitingForResponse = false;
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

bool HouseholdBidderRole::BidUnit() {
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
            float bidValue = maxSurplus + CalculateWP();
            if (owner && bidValue > 0.0f && unit->IsAvailable()) {
                owner->Post(LTMID_BID, GetParent(),
                        new BidMessage(Bid(unit->GetId(), GetParent()->getId(),
                        bidValue)));
                return true;
            }
        }
    }
    return false;
}

float HouseholdBidderRole::CalculateSurplus(const Unit& unit) {
    float askingPrice = unit.GetHedonicPrice(); //needs to be reviewed by Victor.
    float wp = CalculateWP();
    float b = (askingPrice + (float) pow(askingPrice - wp, 0.5));
    return (float) ((pow(b, 2) - wp * b) / (-askingPrice + b));
}

float HouseholdBidderRole::CalculateWP() {
    return (float) ((hh->GetIncome() * 1.0f) +
            (hh->GetNumberOfMembers() * 2.0f));
    return .0f;
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