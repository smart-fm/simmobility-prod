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
#include "entities/Agent_LT.hpp"
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


AgentLifeCycleEventArgsLT::AgentLifeCycleEventArgsLT(unsigned int agentId, Agent_LT* agent): agentId(agentId), agent(agent) {}

AgentLifeCycleEventArgsLT::~AgentLifeCycleEventArgsLT() {}

AgentLifeCycleEventArgsLT& AgentLifeCycleEventArgsLT::operator=(const AgentLifeCycleEventArgsLT& source)
{
    EventArgs::operator =(source);
    this->agentId = source.agentId;
    return *this;
}

unsigned int AgentLifeCycleEventArgsLT::GetAgentId() const
{
    return agentId;
}

sim_mob::Agent_LT* AgentLifeCycleEventArgsLT::GetAgent() const
{
    return agent;
}

