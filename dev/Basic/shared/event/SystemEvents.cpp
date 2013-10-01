//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * File:   SystemEvents.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on August 29, 2013, 3:26 PM
 */

#include "SystemEvents.hpp"
#include "entities/Agent.hpp"
using namespace sim_mob::event;


AgentLifeCycleEventArgs::AgentLifeCycleEventArgs(unsigned int agentId, Agent* agent)
: agentId(agentId), agent(agent) {
}

AgentLifeCycleEventArgs::~AgentLifeCycleEventArgs() {
}

AgentLifeCycleEventArgs& AgentLifeCycleEventArgs::operator=(const AgentLifeCycleEventArgs& source) {
    EventArgs::operator =(source);
    this->agentId = source.agentId;
    return *this;
}

unsigned int AgentLifeCycleEventArgs::GetAgentId() const {
    return agentId;
}

sim_mob::Agent* AgentLifeCycleEventArgs::GetAgent() const {
    return agent;
}
