/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   LT_Agent.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on March 13, 2013, 6:36 PM
 */

#include "LT_Agent.hpp"
#include "conf/simpleconf.hpp"
#include "workers/Worker.hpp"
#include "role/Bidder.hpp"
#include "role/Seller.hpp"

using namespace sim_mob;
using namespace long_term;

LT_Agent::LT_Agent(int id, HousingMarket* market, float income,
        int numberOfMembers)
: Agent(ConfigParams::GetInstance().mutexStategy, id), market(market),
UnitHolder(id), currentRole(nullptr), income(income),
numberOfMembers(numberOfMembers) {

    if (getId() % 2 == 0) {
        currentRole = new Seller(this, market);
    } else {
        currentRole = new Bidder(this, market);
    }
}

LT_Agent::~LT_Agent() {
}

void LT_Agent::load(const map<string, string>& configProps) {
}

EventManager& LT_Agent::GetEventManager() {
    currWorker->GetEventManager();
}

bool LT_Agent::frame_init(timeslice now) {
    return true;
}

Entity::UpdateStatus LT_Agent::frame_tick(timeslice now) {
    if (currentRole) {
        do {
            currentRole->Update(now);
        } while (ReadMessage());
    }
    return Entity::UpdateStatus(UpdateStatus::RS_CONTINUE);
}

void LT_Agent::frame_output(timeslice now) {
}

bool LT_Agent::isNonspatial() {
    return false;
}

void LT_Agent::HandleMessage(MessageType type, MessageReceiver& sender,
        const Message& message) {
    if (currentRole) {
        currentRole->HandleMessage(type, sender, message);
    }
}

float LT_Agent::GetIncome() const {
    return income;
}

float LT_Agent::GetNumberOfMembers() const {
    return numberOfMembers;
}