//Copyright (c) 2014 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "Handlers.hpp"

#include "entities/commsim/message/Messages.hpp"
#include "entities/commsim/serialization/CommsimSerializer.hpp"


sim_mob::HandlerLookup::HandlerLookup() 
{
	//Register all known handlers.
	defaultHandlerMap["MULTICAST"] = new sim_mob::MulticastHandler();
	defaultHandlerMap["UNICAST"] = new sim_mob::UnicastHandler();
	defaultHandlerMap["ALL_LOCATIONS_DATA"] = new sim_mob::AllLocationHandler();
	defaultHandlerMap["AGENTS_INFO"] = new sim_mob::AgentsInfoHandler();
	defaultHandlerMap["TIME_DATA"] = new sim_mob::NullHandler();
	defaultHandlerMap["READY_TO_RECEIVE"] = new sim_mob::NullHandler();
	defaultHandlerMap["WHOAMI"] = new sim_mob::NullHandler();
	defaultHandlerMap["CLIENT_MESSAGES_DONE"] = new sim_mob::NullHandler();
	defaultHandlerMap["REMOTE_LOG"] = new sim_mob::RemoteLogHandler();
	defaultHandlerMap["REROUTE_REQUEST"] = new sim_mob::RerouteRequestHandler();
	defaultHandlerMap["NEW_CLIENT"] = new sim_mob::NewClientHandler();
}

sim_mob::HandlerLookup::~HandlerLookup() 
{
	//Reclaim handlers
	for (std::map<std::string, const sim_mob::Handler*>::const_iterator it=defaultHandlerMap.begin(); it!=defaultHandlerMap.end(); it++) {
		delete it->second;
	}
	for (std::map<std::string, const sim_mob::Handler*>::const_iterator it=customHandlerMap.begin(); it!=customHandlerMap.end(); it++) {
		delete it->second;
	}
	defaultHandlerMap.clear();
	customHandlerMap.clear();
}


const sim_mob::Handler* sim_mob::HandlerLookup::getHandler(const std::string& msgType) const
{
	std::map<std::string, const sim_mob::Handler*>::const_iterator it = customHandlerMap.find(msgType);
	if (it!=customHandlerMap.end()) {
		return it->second;
	}
	it = defaultHandlerMap.find(msgType);
	if (it!=defaultHandlerMap.end()) {
		return it->second;
	}

	throw std::runtime_error("Unknown handler for message type.");
}

void sim_mob::HandlerLookup::addHandlerOverride(const std::string& mType, const Handler* handler)
{
	if (customHandlerMap.find(mType) != customHandlerMap.end()) {
		throw std::runtime_error("Custom handler type has already been registered.");
	}
	customHandlerMap[mType] = handler;
}


void sim_mob::AgentsInfoHandler::handle(boost::shared_ptr<ConnectionHandler> handler, const MessageConglomerate& messages, int msgNumber, Broker* broker) const
{
	//Ask the serializer for an AgentsInfo message.
	AgentsInfoMessage aInfo = CommsimSerializer::parseAgentsInfo(messages, msgNumber);

	//TODO: React
}


void sim_mob::AllLocationHandler::handle(boost::shared_ptr<ConnectionHandler> handler, const MessageConglomerate& messages, int msgNumber, Broker* broker) const
{
	//Ask the serializer for an AllLocations message.
	AllLocationsMessage aInfo = CommsimSerializer::parseAllLocations(messages, msgNumber);

	//TODO: React
}



void sim_mob::UnicastHandler::handle(boost::shared_ptr<ConnectionHandler> handler, const MessageConglomerate& messages, int msgNumber, Broker* broker) const
{
	//Ask the serializer for a Unicast message.
	UnicastMessage ucMsg = CommsimSerializer::parseUnicast(messages, msgNumber);

	throw std::runtime_error("The default UnicastHandler is currently unused by Sim Mobility (we can and should probably change it to the Android-only one).");
}


void sim_mob::MulticastHandler::handle(boost::shared_ptr<ConnectionHandler> handler, const MessageConglomerate& messages, int msgNumber, Broker* broker) const
{
	//Ask the serializer for a Multicast message.
	MulticastMessage mcMsg = CommsimSerializer::parseMulticast(messages, msgNumber);

	throw std::runtime_error("The default MulticastHandler is currently unused by Sim Mobility (we can and should probably change it to the Android-only one).");
}

void sim_mob::RemoteLogHandler::handle(boost::shared_ptr<ConnectionHandler> handler, const MessageConglomerate& messages, int msgNumber, Broker* broker) const
{
	//Ask the serializer for a RemoteLog message.
	RemoteLogMessage rmMsg = CommsimSerializer::parseRemoteLog(messages, msgNumber);

	//Now we can just log it.
	//At the moment, we are so many levels removed from Broker that we'll just put it on stdout.
	//Ideally, it would (eventually) go into out.txt.
	Print() <<"Client [" <<rmMsg.sender_id <<"] relayed remote log message: \"" <<rmMsg.logMessage <<"\"\n";
}

void sim_mob::RerouteRequestHandler::handle(boost::shared_ptr<ConnectionHandler> handler, const MessageConglomerate& messages, int msgNumber, Broker* broker) const
{
	//Ask the serializer for a RerouteRequest message.
	RerouteRequestMessage rmMsg = CommsimSerializer::parseRerouteRequest(messages, msgNumber);

	//Does this agent exist?
	boost::shared_ptr<sim_mob::ClientHandler> agentHandle;
	if(!broker->getClientHandler(rmMsg.sender_id, rmMsg.sender_type, agentHandle)) {
		WarnOut("RerouteRequest can't find Agent (self)." << std::endl);
		return;
	}

	//Double-check Agent validity.
	if(!agentHandle->agent) {
		WarnOut("RerouteRequest found invalid Agent; may have completed route already." << std::endl);
		return;
	}

	//Now dispatch through the MessageBus.
	sim_mob::messaging::MessageBus::PublishEvent(sim_mob::event::EVT_CORE_COMMSIM_REROUTING_REQUEST,
		agentHandle->agent, messaging::MessageBus::EventArgsPtr(new event::ReRouteEventArgs(rmMsg.blacklistRegion))
	);
}

void sim_mob::NewClientHandler::handle(boost::shared_ptr<ConnectionHandler> handler, const MessageConglomerate& messages, int msgNumber, Broker* broker) const
{
	//Ask the serializer for a NewClient message.
	NewClientMessage rmMsg = CommsimSerializer::parseNewClient(messages, msgNumber);

	//Query this agent's ID; tell the Broker that we are expecting a reply.
	WhoAreYouProtocol::QueryAgentAsync(handler, *broker);
}






