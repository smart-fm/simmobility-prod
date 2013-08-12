/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   BidderParams.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on March 11, 2013, 3:05 PM
 */

#include "BidderParams.hpp"
#include "util/Utils.hpp"

using namespace sim_mob::long_term;

BidderParams::BidderParams(BigSerial householdId, double hhIncomeWeight, double unitTypeWeight,
        double unitAreaWeight, double unitStoreyWeight, double unitRentWeight,
        double urgencyToBuy, double priceQuality) :
householdId(householdId), unitTypeWeight(unitTypeWeight),
unitStoreyWeight(unitStoreyWeight), unitAreaWeight(unitAreaWeight),
unitRentWeight(unitRentWeight), urgencyToBuy(urgencyToBuy), 
hhIncomeWeight(hhIncomeWeight), priceQuality(priceQuality) {
}

BidderParams::~BidderParams() {
}

BidderParams& BidderParams::operator=(const BidderParams& source) {
    this->householdId = source.householdId;
    this->unitAreaWeight = source.unitAreaWeight;
    this->unitRentWeight = source.unitRentWeight;
    this->unitStoreyWeight = source.unitStoreyWeight;
    this->unitTypeWeight = source.unitTypeWeight;
    this->urgencyToBuy = source.urgencyToBuy;
    this->hhIncomeWeight = source.hhIncomeWeight;
    this->priceQuality = source.priceQuality;
    return *this;
}

BigSerial BidderParams::GetHouseholdId() const {
    return householdId;
}

double BidderParams::GetHH_IncomeWeight() const {
    return hhIncomeWeight;
}

double BidderParams::GetUnitTypeWeight() const {
    return unitTypeWeight;
}

double BidderParams::GetUnitStoreyWeight() const {
    return unitStoreyWeight;
}

double BidderParams::GetUnitAreaWeight() const {
    return unitAreaWeight;
}

double BidderParams::GetUnitRentWeight() const {
    return unitRentWeight;
}

double BidderParams::GetUrgencyToBuy() const {
    return urgencyToBuy;
}

double BidderParams::GetPriceQuality() const {
    return priceQuality;
}