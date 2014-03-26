//Copyright (c) 2014 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "Handlers.hpp"

#include "entities/commsim/message/Messages.hpp"
#include "entities/commsim/serialization/CommsimSerializer.hpp"


sim_mob::HandlerLookup::HandlerLookup() 
{
	//Register all known handlers.
	HandlerMap["MULTICAST"] = new sim_mob::MulticastHandler();
	HandlerMap["UNICAST"] = new sim_mob::UnicastHandler();
	HandlerMap["ALL_LOCATIONS_DATA"] = new sim_mob::AllLocationHandler();
	HandlerMap["AGENTS_INFO"] = new sim_mob::AgentsInfoHandler();
	HandlerMap["TIME_DATA"] = new sim_mob::NullHandler();
	HandlerMap["READY_TO_RECEIVE"] = new sim_mob::NullHandler();
}

sim_mob::HandlerLookup::~HandlerLookup() 
{
	//Reclaim handlers
	for (std::map<std::string, const sim_mob::Handler*>::const_iterator it=HandlerMap.begin(); it!=HandlerMap.end(); it++) {
		delete it->second;
	}
	HandlerMap.clear();
}


const sim_mob::Handler* sim_mob::HandlerLookup::getHandler(const std::string& msgType)
{
	std::map<std::string, const sim_mob::Handler*>::const_iterator it = HandlerMap.find(msgType);
	if (it!=HandlerMap.end()) {
		return it->second;
	}

	throw std::runtime_error("Unknown handler for message type.");
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

	//TODO: React
}


void sim_mob::MulticastHandler::handle(boost::shared_ptr<ConnectionHandler> handler, const MessageConglomerate& messages, int msgNumber, Broker* broker) const
{
	//Ask the serializer for a Multicast message.
	MulticastMessage mcMsg = CommsimSerializer::parseMulticast(messages, msgNumber);

	//TODO: React
}


