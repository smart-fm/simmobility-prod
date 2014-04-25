//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   SystemEvents.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on August 29, 2013, 11:30 AM
 */

#pragma once

#include "args/EventArgs.hpp"
#include "event/EventListener.hpp"

namespace sim_mob {
class Agent;

namespace event {

enum {
	EVT_CORE_START    = 100000,
	EVT_SHORT_START   = 200000,
	EVT_MEDIUM_START  = 300000,
	EVT_LONG_START    = 400000,
};
        
enum CoreEvent {
	EVT_CORE_SYTEM_START = EVT_CORE_START,
	EVT_CORE_ROLE_INIT,

	//agent life cycle.
	EVT_CORE_AGENT_DIED,
	EVT_CORE_AGENT_UPDATED,

	//Called when the commsim Broker establishes a connection with an Agent.
	//NOTE: This only happens in the short-term, but it is registered at the Agent level, so it must be in "shared".
	EVT_CORE_COMMSIM_ENABLED_FOR_AGENT,

	//Called when the commsim Broker forwards a rerouting request. For now, this only comes in from an Android-emulated entity.
	EVT_CORE_COMMSIM_REROUTING_REQUEST,

	// amod reroute event with new path
	EVT_AMOD_REROUTING_REQUEST_WITH_PATH,
	// amod reroute event with only start node, end node, driver generate shortest path
	EVT_AMOD_REROUTING_REQUEST_WITH_ORI_DEST
};

enum CoreContext {
	CXT_CORE_AGENT_UPDATE
};


///Agent Life Cycle event args (no documentation provided).
class AgentLifeCycleEventArgs : public EventArgs {
public:
	AgentLifeCycleEventArgs(unsigned int agentId, Agent* agent);
	virtual ~AgentLifeCycleEventArgs();

	virtual AgentLifeCycleEventArgs& operator=(const AgentLifeCycleEventArgs& source);
	unsigned int GetAgentId() const;
	Agent* GetAgent() const;

private:
	unsigned int agentId;
	Agent * agent;
};

}}
