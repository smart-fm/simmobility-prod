/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   UnitHolder.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on March 12, 2013, 2:36 PM
 */

#include "UnitHolder.hpp"
#include "util/LangHelpers.hpp"

using std::map;
using std::pair;
using namespace sim_mob;
using namespace sim_mob::long_term;

int UnitHolder::unitX = 1;

UnitHolder::UnitHolder(int id) : LT_Agent(id), firstTime(true) {
    //this->bidderRole = new Bidder(this);
    //this->sellerRole = new Seller(this);
}

UnitHolder::~UnitHolder() {
    for (HoldingUnits::iterator itr = holdingUnits.begin();
            itr != holdingUnits.end(); itr++) {
        Unit* unit = (*itr).second;
        safe_delete_item(unit);
    }
    holdingUnits.clear();
    safe_delete_item(role);
}

bool UnitHolder::Update(timeslice now) {
    
    if (firstTime){
        this->role =  (getId() % 2 == 0) ? static_cast<Role*>(new Bidder(this)) : static_cast<Role*>(new Seller(this));
        firstTime = false;
    }
    role->Update(now);
    return true;
}

bool UnitHolder::AddUnit(Unit* unit) {
    if (unit && !HasUnit(unit->GetId())) {
        holdingUnits.insert(HoldingUnitsEntry(unit->GetId(), unit));
        //change the owner of the unit.
        //unit->owner = this;
        return true;
    }
    return false;
}

Unit* UnitHolder::RemoveUnit(UnitId id) {
    Unit* retVal = nullptr;
    if (HasUnit(id)) {
        Unit* retVal = GetById(id);
        if (retVal) {
            holdingUnits.erase(id);
        }
        // removes this holder as owner.
        //unit->owner = NULL;
    }
    return retVal;
}

bool UnitHolder::HasUnit(UnitId id) const {
    return (holdingUnits.find(id) != holdingUnits.end());
}

Unit* UnitHolder::GetById(UnitId id) {
    HoldingUnits::iterator mapItr = holdingUnits.find(id);
    if (mapItr != holdingUnits.end()) {
        return mapItr->second;
    }
    return nullptr;
}

void UnitHolder::GetUnits(list<Unit*>& outUnits) {
    for (HoldingUnits::iterator itr = holdingUnits.begin();
            itr != holdingUnits.end(); itr++) {
        Unit* unit = (*itr).second;
        if (unit) {
            outUnits.push_back(unit);
        }
    }
}
