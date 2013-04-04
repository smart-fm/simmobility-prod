/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   LT_EventArgs.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on April 4, 2013, 5:42 PM
 */

#include "LT_EventArgs.h"

using namespace sim_mob;
using namespace sim_mob::long_term;

BidEventArgs::BidEventArgs(int bidderId, double bid) : bidValue(bid),
bidderId(bidderId), response(NOT_AVAILABLE) {
}

BidEventArgs::BidEventArgs(int bidderId, BidResponse response) : bidValue(0),
bidderId(bidderId), response(response) {
}

BidEventArgs::BidEventArgs(const BidEventArgs& orig) : bidValue(orig.bidValue),
bidderId(orig.bidderId), response(orig.response) {
}

BidEventArgs::~BidEventArgs() {
}

const BidResponse BidEventArgs::GetResponse() const {
    return response;
}

const double BidEventArgs::GetBid() const {
    return bidValue;
}

const int BidEventArgs::GetBidderId()const {
    return bidderId;
}