/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   HousingMarket.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on March 11, 2013, 4:13 PM
 */

#include "HousingMarket.hpp"

using namespace sim_mob;
using namespace sim_mob::long_term;

HousingMarket::HousingMarket() : UnitHolder(-1) {
}

HousingMarket::~HousingMarket() {
}

bool HousingMarket::Add(Unit* unit, bool reOwner) {
    // no re-parent the unit to the market.
    return UnitHolder::Add(unit, false);
}

Unit* HousingMarket::Remove(UnitId id, bool reOwner) {
    // no re-parent the unit to the market.
    return UnitHolder::Remove(id, false);
}

