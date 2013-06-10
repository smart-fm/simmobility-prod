/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   Role.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on April 4, 2013, 4:55 PM
 */

#include "LT_Role.hpp"
#include "agent/LT_Agent.hpp"

using namespace sim_mob;
using namespace sim_mob::long_term;

LT_Role::LT_Role(LT_Agent* parent) : active(true), parent(parent) {
}

LT_Role::~LT_Role() {
}

bool LT_Role::isActive() const {
    return active;
}

void LT_Role::SetActive(bool active) {
    this->active = active;
}

LT_Agent* LT_Role::GetParent() const{
    return this->parent;
}

void LT_Role::HandleMessage(MessageType type, MessageReceiver& sender, const Message& message){

}