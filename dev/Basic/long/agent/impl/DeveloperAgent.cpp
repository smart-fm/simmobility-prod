//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * File:   DeveloperAgent.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on Mar 5, 2014, 6:36 PM
 */

#include "DeveloperAgent.hpp"
#include "message/MessageBus.hpp"
#include "role/LT_Role.hpp"
#include "database/entity/Developer.hpp"
#include "model/DeveloperModel.hpp"

using namespace sim_mob::long_term;
using namespace sim_mob::event;
using namespace sim_mob::messaging;
using sim_mob::Entity;
using std::vector;
using std::string;
using std::map;
using std::endl;

DeveloperAgent::DeveloperAgent(Developer* developer, DeveloperModel* model)
: LT_Agent((developer) ? developer->getId() : INVALID_ID), model(model) {
}

DeveloperAgent::~DeveloperAgent() {
}

void DeveloperAgent::assignParcel(BigSerial parcelId) {
    if (parcelId != INVALID_ID) {
        parcelsToProcess.push_back(parcelId);
    }
}

bool DeveloperAgent::onFrameInit(timeslice now) {
    return true;
}

Entity::UpdateStatus DeveloperAgent::onFrameTick(timeslice now) {
    return Entity::UpdateStatus(UpdateStatus::RS_CONTINUE);
}

void DeveloperAgent::onFrameOutput(timeslice now) {
}

void DeveloperAgent::onEvent(EventId eventId, Context ctxId,
        EventPublisher*, const EventArgs& args) {
}

void DeveloperAgent::onWorkerEnter() {
}

void DeveloperAgent::onWorkerExit() {
}

void DeveloperAgent::HandleMessage(Message::MessageType type,
        const Message& message) {
}
