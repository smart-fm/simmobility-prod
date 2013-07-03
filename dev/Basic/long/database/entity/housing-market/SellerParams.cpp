/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   SellerParams.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on March 11, 2013, 3:05 PM
 */

#include "SellerParams.hpp"
#include "util/Utils.hpp"

using namespace sim_mob::long_term;

SellerParams::SellerParams(BigSerial householdId, double priceImportance, 
        double expectedEvents) : priceImportance(priceImportance), 
        expectedEvents(expectedEvents) {
}

SellerParams::~SellerParams() {
}

SellerParams& SellerParams::operator=(const SellerParams& source) {
    this->householdId = source.householdId;
    this->priceImportance = source.priceImportance;
    this->expectedEvents = source.expectedEvents;
    return *this;
}

BigSerial SellerParams::GetHouseholdId() const {
    return householdId;
}

double SellerParams::GetPriceImportance() const {
    return priceImportance;
}

double SellerParams::GetExpectedEvents() const {
    return expectedEvents;
}