/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   Seller.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on April 4, 2013, 5:13 PM
 */
#include <math.h>
#include "Seller.hpp"
#include "message/LT_Message.hpp"

using namespace sim_mob;
using namespace sim_mob::long_term;

Seller::Seller(LT_Agent* parent, HousingMarket* market)
: LT_Role(parent), market(market) {
}

Seller::~Seller() {
}

void Seller::Update(timeslice currTime) {
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

void Seller::HandleMessage(MessageType type, MessageReceiver& sender,
        const Message& message) {

    switch (type) {
        case LTID_BID:// Bid received 
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
            sender.Post(LTID_BID_RSP, GetParent(),
                    new BidMessage(Bid(msg->GetBid()),
                    ((decision) ? ACCEPTED : NOT_ACCEPTED)));
            break;
        }

        default:break;
    }
}

bool Seller::Decide(const Bid& bid, const Unit& unit) {
    return bid.GetValue() > unit.GetReservationPrice();
}

void Seller::AdjustUnitParams(Unit& unit) {
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

