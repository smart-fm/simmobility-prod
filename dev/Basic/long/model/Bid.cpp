/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   Bid.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on April 5, 2013, 5:03 PM
 */

#include "Bid.hpp"

using namespace sim_mob;
using namespace sim_mob::long_term;

Bid::Bid(UnitId unitId, int bidderId, float value)
: unitId(unitId), bidderId(bidderId), value(value) {
}

Bid::Bid(const Bid& source) {
    this->unitId = source.unitId;
    this->bidderId = source.bidderId;
    this->value = source.value;
}

Bid::~Bid() {
}

Bid& Bid::operator=(const Bid& source) {
    this->unitId = source.unitId;
    this->bidderId = source.bidderId;
    this->value = source.value;
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
