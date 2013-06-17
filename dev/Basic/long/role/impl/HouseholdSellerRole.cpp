/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   HouseholdSellerRole.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on May 16, 2013, 5:13 PM
 */
#include <math.h>
#include "HouseholdSellerRole.hpp"
#include "message/LT_Message.hpp"
#include "agent/impl/HouseholdAgent.hpp" 

using namespace sim_mob;
using namespace sim_mob::long_term;

HouseholdSellerRole::HouseholdSellerRole(HouseholdAgent* parent, Household* hh,
        HousingMarket* market)
: LT_AgentRole(parent), market(market), hh(hh), currentTime(0, 0), hasUnitsToSale(true) {
}

HouseholdSellerRole::~HouseholdSellerRole() {
    maxBidsOfDay.clear();
}

void HouseholdSellerRole::Update(timeslice now) {
    if (hasUnitsToSale) {
        list<Unit*> units;
        GetParent()->GetUnits(units);
        for (list<Unit*>::iterator itr = units.begin(); itr != units.end();
                itr++) {
            if ((*itr)->IsAvailable()) {
                market->AddUnit((*itr));
            }
        }
        hasUnitsToSale = false;
    }
    if (now.ms() > currentTime.ms()) {
        // Day has changed we need to notify the last day winners.
        NotifyWinnerBidders();
        AdjustNotSelledUnits();
    }
    currentTime = now;
}

void HouseholdSellerRole::HandleMessage(MessageType type, MessageReceiver& sender,
        const Message& message) {

    switch (type) {
        case LTMID_BID:// Bid received 
        {
            BidMessage* msg = MSG_CAST(BidMessage, message);
            Unit* unit = GetParent()->GetUnitById(msg->GetBid().GetUnitId());
            LogOut("Seller: [" << GetParent()->getId() <<
                    "] received a bid: " << msg->GetBid() <<
                    " at day: " << currentTime.ms() << endl);
            bool decision = false;
            if (unit && unit->IsAvailable()) {
                //verify if is the bid satisfies the asking price.
                decision = Decide(msg->GetBid(), *unit);
                if (decision) {
                    //get the maximum bid of the day
                    Bids::iterator bidItr = maxBidsOfDay.find(unit->GetId());
                    Bid* maxBidOfDay = nullptr;
                    if (bidItr != maxBidsOfDay.end()) {
                        maxBidOfDay = &(bidItr->second);
                    }

                    if (!maxBidOfDay) {
                        maxBidsOfDay.insert(BidEntry(unit->GetId(),
                                msg->GetBid()));
                    } else if (maxBidOfDay->GetValue() < msg->GetBid().GetValue()) {
                        // bid is higher than the current one of the day.
                        // it is necessary to notify the old max bidder
                        // that his bid was not accepted.
                        //reply to sender.
                        maxBidOfDay->GetBidder()->Post(LTMID_BID_RSP, GetParent(),
                                new BidMessage(Bid(*maxBidOfDay), BETTER_OFFER));
                        maxBidsOfDay.erase(unit->GetId());
                        //update the new bid and bidder.
                        maxBidsOfDay.insert(BidEntry(unit->GetId(), msg->GetBid()));
                    } else {
                        sender.Post(LTMID_BID_RSP, GetParent(),
                                new BidMessage(Bid(msg->GetBid()), BETTER_OFFER));
                    }
                } else {
                    sender.Post(LTMID_BID_RSP, GetParent(),
                            new BidMessage(Bid(msg->GetBid()), NOT_ACCEPTED));
                }
            }
            break;
        }
        default:break;
    }
}

bool HouseholdSellerRole::Decide(const Bid& bid, const Unit& unit) {
    return bid.GetValue() > unit.GetReservationPrice();
}

void HouseholdSellerRole::AdjustUnitParams(Unit& unit) {
    float denominator = pow((1 - 2 * unit.GetFixedCost()), 0.5f);
    //re-calculates the new reservation price.
    float reservationPrice =
            (unit.GetReservationPrice() + unit.GetFixedCost()) / denominator;
    //re-calculates the new hedonic price.
    float hedonicPrice = reservationPrice / denominator;
    //update values.
    unit.SetReservationPrice(reservationPrice);
    unit.SetHedonicPrice(hedonicPrice);
}

void HouseholdSellerRole::NotifyWinnerBidders() {

    for (Bids::iterator itr = maxBidsOfDay.begin(); itr != maxBidsOfDay.end();
            itr++) {
        Bid* maxBidOfDay = &(itr->second);
        maxBidOfDay->GetBidder()->Post(LTMID_BID_RSP, GetParent(),
                new BidMessage(Bid(*maxBidOfDay), ACCEPTED));
        Unit* unit = GetParent()->GetUnitById(maxBidOfDay->GetUnitId());
        if (unit && unit->IsAvailable()) {
            unit->SetAvailable(false);
            unit = GetParent()->RemoveUnit(unit->GetId());
        }
    }
    // notify winners.
    maxBidsOfDay.clear();
}

void HouseholdSellerRole::AdjustNotSelledUnits() {
    list<Unit*> units;
    GetParent()->GetUnits(units);
    for (list<Unit*>::iterator itr = units.begin(); itr != units.end(); itr++) {
        if ((*itr)->IsAvailable()) {
            AdjustUnitParams((**itr));
        }
    }
}
