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
            PrintOut("Household ["<< id<<"] holds the Unit ["<< unit->getId() <<"]" << std::endl);
            safe_delete_item(unit);
        }
    }
    holdingUnits.clear();
}

bool UnitHolder::addUnit(Unit* unit) {
	boost::upgrade_lock<boost::shared_mutex> up_lock(unitsListMutex);
	boost::upgrade_to_unique_lock<boost::shared_mutex> lock(up_lock);
    return add(unit, true);
}

Unit* UnitHolder::removeUnit(UnitId id) {
	boost::upgrade_lock<boost::shared_mutex> up_lock(unitsListMutex);
	boost::upgrade_to_unique_lock<boost::shared_mutex> lock(up_lock);
    return remove(id, true);
}

bool UnitHolder::hasUnit(UnitId id) const {
	boost::shared_lock<boost::shared_mutex> lock(unitsListMutex);
    return contains(id);
}

Unit* UnitHolder::getUnitById(UnitId id) {
	boost::shared_lock<boost::shared_mutex> lock(unitsListMutex);
    return getById(id);
}

void UnitHolder::getUnits(list<Unit*>& outUnits) {
	boost::shared_lock<boost::shared_mutex> lock(unitsListMutex);
    for (HoldingUnits::iterator itr = holdingUnits.begin();
            itr != holdingUnits.end(); itr++) {
        Unit* unit = (*itr).second;
        if (unit) {
            outUnits.push_back(unit);
        }
    }
}

bool UnitHolder::add(Unit* unit, bool reOwner) {
    if (unit && !contains(unit->getId())) {
        holdingUnits.insert(HoldingUnitsEntry(unit->getId(), unit));
        if (reOwner) {
            unit->setOwner(this);
        }
        return true;
    }
    return false;
}

Unit* UnitHolder::remove(UnitId id, bool reOwner) {
    Unit* retVal = nullptr;
    if (contains(id)) {
        retVal = getById(id);
        if (retVal) {
            holdingUnits.erase(id);
            if (reOwner) {
                retVal->setOwner(nullptr);
            }
        }
    }
    return retVal;
}

bool UnitHolder::contains(UnitId id) const {
    return (holdingUnits.find(id) != holdingUnits.end());
}

Unit* UnitHolder::getById(UnitId id) {
    HoldingUnits::iterator mapItr = holdingUnits.find(id);
    if (mapItr != holdingUnits.end()) {
        return mapItr->second;
    }
    return nullptr;
}
