//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   Bid.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on April 5, 2013, 5:03 PM
 */

#include "Bid.hpp"
#include "metrics/Frame.hpp"

using namespace sim_mob::long_term;

Bid::Bid(BigSerial unitId, BigSerial bidderId, LT_Agent* bidder, double value, 
        timeslice& time, double willingnessToPay, double speculation)
: unitId(unitId), bidderId(bidderId), value(value), time(time), bidder(bidder),
willingnessToPay(willingnessToPay), speculation(speculation){
}

Bid::Bid(const Bid& source) : time(source.time) {
    this->unitId = source.unitId;
    this->bidderId = source.bidderId;
    this->value = source.value;
    this->bidder = source.bidder;
    this->speculation = source.speculation;
    this->willingnessToPay = source.willingnessToPay;
}

Bid::Bid()
: unitId(INVALID_ID), bidderId(INVALID_ID), value(0.0), time(0,0), 
        bidder(nullptr), willingnessToPay(0.0), speculation(0.0) {
}

Bid::~Bid() {
}

Bid& Bid::operator=(const Bid& source) {
    this->unitId = source.unitId;
    this->bidderId = source.bidderId;
    this->value = source.value;
    this->time = source.time;
    this->bidder = source.bidder;
    this->speculation = source.speculation;
    this->willingnessToPay = source.willingnessToPay;
    return *this;
}

BigSerial Bid::getUnitId() const {
    return unitId;
}

BigSerial Bid::getBidderId() const {
    return bidderId;
}

double Bid::getValue() const {
    return value;
}

const timeslice& Bid::getTime() const {
    return time;
}

LT_Agent* Bid::getBidder() const{
    return bidder;
}

double Bid::getWillingnessToPay() const {
    return willingnessToPay;
}

double Bid::getSpeculation() const {
    return speculation;
}

namespace sim_mob {
    namespace long_term {

        std::ostream& operator<<(std::ostream& strm, const Bid& data) {
            return strm << "{"
                    << "\"unitId\":\"" << data.unitId << "\","
                    << "\"bidderId\":\"" << data.bidderId << "\","
                    << "\"value\":\"" << data.value << "\","
                    << "\"day\":\"" << data.time.ms() << "\""
                    << "}";
        }
    }
}