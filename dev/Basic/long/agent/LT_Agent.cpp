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

using namespace sim_mob;
using namespace sim_mob::long_term;
using namespace sim_mob::event;
using namespace sim_mob::messaging;
using std::vector;
using std::string;
using std::map;

LT_Agent::LT_Agent(int id)
: Agent(ConfigParams::GetInstance().mutexStategy, id) {
}

LT_Agent::~LT_Agent() {
}

void LT_Agent::load(const map<string, string>& configProps) {
}

EventManager& LT_Agent::GetEventManager() {
    return currWorkerProvider->getEventManager();
}

bool LT_Agent::frame_init(timeslice now) {
    return OnFrameInit(now);
}

Entity::UpdateStatus LT_Agent::frame_tick(timeslice now) {
    int messageCounter = 0;
    Entity::UpdateStatus status = UpdateStatus::Continue;
    do {
        status = OnFrameTick(now, messageCounter);
        //abort because agent is done.
        if (status.status == UpdateStatus::RS_DONE) {
            break;
        }
        messageCounter++;
    } while (ReadMessage());
    return status;
}

void LT_Agent::frame_output(timeslice now) {
    OnFrameOutput(now);
}

bool LT_Agent::isNonspatial() {
    return false;
}

void LT_Agent::HandleMessage(MessageReceiver::MessageType type, MessageReceiver& sender,
        const Message& message) {
    int x=0;
}
