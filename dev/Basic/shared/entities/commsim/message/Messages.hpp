//Copyright (c) 2014 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "MessageBase.hpp"

#include <string>
#include <vector>
#include <map>

#include "util/DynamicVector.hpp"

namespace sim_mob {

struct UnicastMessage : public sim_mob::MessageBase {
	std::string receiver; ///<Who to send this to.
	UnicastMessage(const MessageBase& base) : MessageBase(base) {}
};

struct MulticastMessage : public sim_mob::MessageBase {
	unsigned int sendingAgent;
	std::vector<unsigned int> recipients;
	std::string msgData;
	MulticastMessage(const MessageBase& base) : MessageBase(base) {}
};

struct AgentsInfoMessage : public sim_mob::MessageBase {
	std::vector<unsigned int> addAgentIds; ///<Agent IDs to add
	std::vector<unsigned int> remAgentIds; ///<Agent IDs to remove
	AgentsInfoMessage(const MessageBase& base) : MessageBase(base) {}
};

struct AllLocationsMessage : public sim_mob::MessageBase {
	std::map<unsigned int, DPoint> agentLocations; ///<Maps agentID=>(x,y) updates for locations.
	AllLocationsMessage(const MessageBase& base) : MessageBase(base) {}
};


}

