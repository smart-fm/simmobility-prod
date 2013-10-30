//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   LT_Agent.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on March 13, 2013, 6:36 PM
 */

#include "LT_Agent.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"

using namespace sim_mob;
using namespace sim_mob::long_term;
using namespace sim_mob::event;
using namespace sim_mob::messaging;
using std::vector;
using std::string;
using std::map;

LT_Agent::LT_Agent(int id)
: Agent(ConfigManager::GetInstance().FullConfig().mutexStategy(), id) {
}

LT_Agent::~LT_Agent() {
}

void LT_Agent::load(const map<string, string>& configProps) {
}

bool LT_Agent::frame_init(timeslice now) {
    return onFrameInit(now);
}

Entity::UpdateStatus LT_Agent::frame_tick(timeslice now) {
    return onFrameTick(now);
}

void LT_Agent::frame_output(timeslice now) {
    onFrameOutput(now);
}

bool LT_Agent::isNonspatial() {
    return false;
}