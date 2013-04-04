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

BidEventArgs::BidEventArgs(double bid) {
    BidArgsValue val;
    val.bid = bid;
    value = val;
}

BidEventArgs::BidEventArgs(BidResponse response){
    BidArgsValue val;
    val.response = response;
    value = val;
}

BidEventArgs::BidEventArgs(const BidEventArgs& orig) : value(orig.value) {
}

BidEventArgs::~BidEventArgs() {
}

const BidResponse BidEventArgs::GetResponse() const{
    return value.response;
}

const double BidEventArgs::GetBid() const{
    return value.bid;
}