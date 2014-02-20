/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   AgentsLookup.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on Feb 18, 2014, 1:32 PM
 */

#include "AgentsLookup.hpp"

using namespace sim_mob;
using namespace sim_mob::long_term;
namespace {

    template <typename T, typename M, typename K>
    inline const T* getById(const M& map, const K& key) {
        typename M::const_iterator itr = map.find(key);
        if (itr != map.end()) {
            return (*itr).second;
        }
        return nullptr;
    }
}

AgentsLookup::AgentsLookup() {
}

AgentsLookup::~AgentsLookup() {
    reset();
}

void AgentsLookup::reset() {
    householdsById.clear();
}

void AgentsLookup::addHousehold(const HouseholdAgent* agent) {
    if (agent && !getById<HouseholdAgent>(householdsById, agent->GetId())) {
        householdsById.insert(std::make_pair(agent->GetId(), agent));
    }
}

const HouseholdAgent* AgentsLookup::getHouseholdById(const BigSerial id) const {
    return getById<HouseholdAgent>(householdsById, id);
}