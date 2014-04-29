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

///A Uni/Multicast-style opaque message, sent from ONE agent to ONE other agent *or* broadcast to anyone in range.
///This struct carries the information from the first agent to Sim Mobility (which may relay it to ns-3).
///Note that from/to IDs here represent the end-goal agents (e.g., from emulator#1 to emulator#2), NOT the communication sender/destination.
struct OpaqueSendMessage : public sim_mob::MessageBase {
	std::string fromId; ///<The Agent sending this message.
	std::vector<std::string> toIds; ///<The Agent(s) we are sending this message to.
	std::string format; ///<The way we format the data string.
	bool broadcast; ///<If true, Sim Mobility will overwrite "toIds" with the nearest Agents (using the Aura Manager).
	std::string data; ///<The actual message data
	OpaqueSendMessage(const MessageBase& base) : MessageBase(base) {}
};

///A Uni/Multicast-style opaque message, sent from ONE agent to ONE other agent.
///This struct carries the information from (optionally) ns-3 to Sim Mobility and (always) Sim Mobility to the recipient..
///Note that from/to IDs here represent the end-goal agents (e.g., from emulator#1 to emulator#2), NOT the communication sender/destination.
struct OpaqueReceiveMessage : public sim_mob::MessageBase {
	std::string fromId; ///<The Agent sending this message.
	std::string toId; ///<The Agent we are sending this message to.
	std::string format; ///<The way we format the data string.
	std::string data; ///<The actual message data
	OpaqueReceiveMessage(const MessageBase& base) : MessageBase(base) {}
};

struct IdResponseMessage : public sim_mob::MessageBase {
	std::string token;
	std::string id;
	std::string type;
	std::vector<std::string> services;

	IdResponseMessage(const MessageBase& base) : MessageBase(base) {}
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

struct RemoteLogMessage : public sim_mob::MessageBase {
	std::string logMessage; ///<The text being sent.
	RemoteLogMessage(const MessageBase& base) : MessageBase(base) {}
};

struct RerouteRequestMessage : public sim_mob::MessageBase {
	std::string blacklistRegion; ///<The region to avoid.
	RerouteRequestMessage(const MessageBase& base) : MessageBase(base) {}
};

struct TcpConnectMessage : public sim_mob::MessageBase {
	std::string host; ///<The host address to connect to.
	int port; ///<The port to connect over.
	TcpConnectMessage(const MessageBase& base) : MessageBase(base) {}
};

struct TcpDisconnectMessage : public sim_mob::MessageBase {
	std::string host; ///<The host address to disconnect from.
	int port; ///<The port this connection was using.
	TcpDisconnectMessage(const MessageBase& base) : MessageBase(base) {}
};



}

