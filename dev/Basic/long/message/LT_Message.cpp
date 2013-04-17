/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   LT_Message.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on April 8, 2013, 11:06 AM
 */

#include "LT_Message.hpp"

using namespace sim_mob;
using namespace sim_mob::long_term;

BidMessage::BidMessage(const Bid& bid)
: bid(bid), response(NOT_AVAILABLE) {
}

BidMessage::BidMessage(const Bid& bid, BidResponse response)
: bid(bid), response(response) {
}

BidMessage::BidMessage(const BidMessage& source)
: bid(source.bid), response(source.response) {
}

BidMessage::~BidMessage() {
}

BidMessage& BidMessage::operator=(const BidMessage& source) {
    this->bid = source.bid;
    this->response = source.response;
    return *this;
}

const BidResponse BidMessage::GetResponse() const {
    return response;
}

const Bid& BidMessage::GetBid() const {
    return bid;
}
