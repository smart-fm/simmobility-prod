/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   Bid.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on April 5, 2013, 5:03 PM
 */

#include "Bid.hpp"
#include "metrics/Frame.hpp"

using namespace sim_mob::long_term;
using namespace sim_mob::messaging;

Bid::Bid(UnitId unitId, int bidderId, MessageHandler* bidder, float value, timeslice& time)
: unitId(unitId), bidderId(bidderId), value(value), time(time), bidder(bidder) {
}

Bid::Bid(const Bid& source) : time(source.time) {
    this->unitId = source.unitId;
    this->bidderId = source.bidderId;
    this->value = source.value;
    this->bidder = source.bidder;
}

Bid::Bid()
: unitId(INVALID_ID), bidderId(INVALID_ID), value(.0f), time(0,0), 
        bidder(nullptr) {
}

Bid::~Bid() {
}

Bid& Bid::operator=(const Bid& source) {
    this->unitId = source.unitId;
    this->bidderId = source.bidderId;
    this->value = source.value;
    this->time = source.time;
    this->bidder = source.bidder;
    return *this;
}

UnitId Bid::GetUnitId() const {
    return unitId;
}

int Bid::GetBidderId() const {
    return bidderId;
}

float Bid::GetValue() const {
    return value;
}

const timeslice& Bid::GetTime() const {
    return time;
}

MessageHandler* Bid::GetBidder() const{
    return bidder;
}
