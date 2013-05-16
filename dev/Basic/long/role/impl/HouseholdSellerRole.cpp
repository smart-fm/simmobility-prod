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
: LT_AgentRole(parent), market(market), hh(hh) {
}

HouseholdSellerRole::~HouseholdSellerRole() {
}

void HouseholdSellerRole::Update(timeslice currTime) {
    if (isActive()) {
        list<Unit*> units;
        GetParent()->GetUnits(units);
        for (list<Unit*>::iterator itr = units.begin(); itr != units.end();
                itr++) {
            if ((*itr)->IsAvailable()) {
                market->AddUnit((*itr));
            }
        }
        SetActive(false);
    }
}

void HouseholdSellerRole::HandleMessage(MessageType type, MessageReceiver& sender,
        const Message& message) {

    switch (type) {
        case LTMID_BID:// Bid received 
        {
            BidMessage* msg = MSG_CAST(BidMessage, message);
            Unit* unit = GetParent()->GetUnitById(msg->GetBid().GetUnitId());
            LogOut("Agent: " << GetParent()->getId() << " received a bid: "
                    << msg->GetBid().GetValue() << " from: "
                    << msg->GetBid().GetBidderId() << " to the Unit: "
                    << msg->GetBid().GetUnitId() << endl);
            bool decision = false;
            if (unit && unit->IsAvailable()) {
                //take a decision.
                decision = Decide(msg->GetBid(), *unit);
                if (decision) {
                    unit->SetAvailable(false);
                    unit = GetParent()->RemoveUnit(msg->GetBid().GetUnitId());
                } else {
                    AdjustUnitParams(*unit);
                }
            }
            //reply to sender.
            sender.Post(LTMID_BID_RSP, GetParent(),
                    new BidMessage(Bid(msg->GetBid()),
                    ((decision) ? ACCEPTED : NOT_ACCEPTED)));
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

