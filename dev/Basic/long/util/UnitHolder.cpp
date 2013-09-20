//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   UnitHolder.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on March 12, 2013, 2:36 PM
 */

#include "UnitHolder.hpp"

using std::map;
using std::pair;
using std::list;
using std::endl;
using namespace sim_mob::long_term;

UnitHolder::UnitHolder(int id) : id(id) {
}

UnitHolder::~UnitHolder() {
    for (HoldingUnits::iterator itr = holdingUnits.begin();
            itr != holdingUnits.end(); itr++) {
        Unit* unit = (*itr).second;
        if (unit && ((unit->owner == nullptr) || (unit->owner && unit->owner == this))) {
            PrintOut("Unit: " << unit->id << " was deleted by: " << id << endl);
            safe_delete_item(unit);
        }
    }
    holdingUnits.clear();
}

bool UnitHolder::AddUnit(Unit* unit) {
	boost::upgrade_lock<boost::shared_mutex> up_lock(unitsListMutex);
	boost::upgrade_to_unique_lock<boost::shared_mutex> lock(up_lock);
    return Add(unit, true);
}

Unit* UnitHolder::RemoveUnit(UnitId id) {
	boost::upgrade_lock<boost::shared_mutex> up_lock(unitsListMutex);
	boost::upgrade_to_unique_lock<boost::shared_mutex> lock(up_lock);
    return Remove(id, true);
}

bool UnitHolder::HasUnit(UnitId id) const {
	boost::shared_lock<boost::shared_mutex> lock(unitsListMutex);
    return Contains(id);
}

Unit* UnitHolder::GetUnitById(UnitId id) {
	boost::shared_lock<boost::shared_mutex> lock(unitsListMutex);
    return GetById(id);
}

void UnitHolder::GetUnits(list<Unit*>& outUnits) {
	boost::shared_lock<boost::shared_mutex> lock(unitsListMutex);
    for (HoldingUnits::iterator itr = holdingUnits.begin();
            itr != holdingUnits.end(); itr++) {
        Unit* unit = (*itr).second;
        if (unit) {
            outUnits.push_back(unit);
        }
    }
}

bool UnitHolder::Add(Unit* unit, bool reOwner) {
    if (unit && !Contains(unit->GetId())) {
        holdingUnits.insert(HoldingUnitsEntry(unit->GetId(), unit));
        if (reOwner) {
            unit->SetOwner(this);
        }
        return true;
    }
    return false;
}

Unit* UnitHolder::Remove(UnitId id, bool reOwner) {
    Unit* retVal = nullptr;
    if (Contains(id)) {
        retVal = GetById(id);
        if (retVal) {
            holdingUnits.erase(id);
            if (reOwner) {
                retVal->SetOwner(nullptr);
            }
        }
    }
    return retVal;
}

bool UnitHolder::Contains(UnitId id) const {
    return (holdingUnits.find(id) != holdingUnits.end());
}

Unit* UnitHolder::GetById(UnitId id) {
    HoldingUnits::iterator mapItr = holdingUnits.find(id);
    if (mapItr != holdingUnits.end()) {
        return mapItr->second;
    }
    return nullptr;
}
