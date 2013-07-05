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

SellerParams::SellerParams(BigSerial householdId, double unitTypeWeight,
        double unitAreaWeight, double unitStoreyWeight, double unitRentWeight, 
        double priceImportance, double expectedEvents) : 
        householdId (householdId), unitTypeWeight(unitTypeWeight),
unitStoreyWeight(unitStoreyWeight), unitAreaWeight(unitAreaWeight),
unitRentWeight(unitRentWeight), priceImportance(priceImportance),
expectedEvents(expectedEvents) {
}

SellerParams::~SellerParams() {
}

SellerParams& SellerParams::operator=(const SellerParams& source) {
    this->householdId = source.householdId;
    this->unitAreaWeight = source.unitAreaWeight;
    this->unitRentWeight = source.unitRentWeight;
    this->unitStoreyWeight = source.unitStoreyWeight;
    this->unitTypeWeight = source.unitTypeWeight;
    this->priceImportance = source.priceImportance;
    this->expectedEvents = source.expectedEvents;
    return *this;
}

BigSerial SellerParams::GetHouseholdId() const {
    return householdId;
}

double SellerParams::GetUnitTypeWeight() const {
    return unitTypeWeight;
}

double SellerParams::GetUnitStoreyWeight() const {
    return unitStoreyWeight;
}

double SellerParams::GetUnitAreaWeight() const {
    return unitAreaWeight;
}

double SellerParams::GetUnitRentWeight() const {
    return unitRentWeight;
}

double SellerParams::GetPriceImportance() const {
    return priceImportance;
}

double SellerParams::GetExpectedEvents() const {
    return expectedEvents;
}