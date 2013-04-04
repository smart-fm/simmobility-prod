/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   Role.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on April 4, 2013, 4:55 PM
 */

#include "Role.hpp"

using namespace sim_mob;
using namespace sim_mob::long_term;

Role::Role(Agent* parent) : active(true), parent(parent) {
}

Role::~Role() {
}

bool Role::isActive() const {
    return active;
}

void Role::SetActive(bool active) {
    this->active = active;
}

Agent* Role::GetParent() const{
    return this->parent;
}
