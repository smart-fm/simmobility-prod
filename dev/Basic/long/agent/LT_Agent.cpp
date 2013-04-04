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
#include <iostream>

using std::cout;
using std::endl;
using namespace sim_mob;
using namespace long_term;

LT_Agent::LT_Agent(int id) : Agent(ConfigParams::GetInstance().mutexStategy, id) {
}

LT_Agent::~LT_Agent() {
}

void LT_Agent::load(const std::map<std::string, std::string>& configProps) {
}

EventManager& LT_Agent::GetEventManager() {
    currWorker->GetEventManager();
}

bool LT_Agent::frame_init(timeslice now) {
    return true;
}

Entity::UpdateStatus LT_Agent::frame_tick(timeslice now) {
    cout << "Time: " << now.ms() << endl;\
    return Update(now) ? Entity::UpdateStatus(UpdateStatus::RS_CONTINUE) : 
        Entity::UpdateStatus(UpdateStatus::RS_DONE);
}

void LT_Agent::frame_output(timeslice now) {
}

bool LT_Agent::isNonspatial(){
    return false;
}