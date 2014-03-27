//Copyright (c) 2014 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "Handlers.hpp"

#include <boost/unordered_map.hpp>

#include "entities/commsim/message/Messages.hpp"
#include "entities/commsim/serialization/CommsimSerializer.hpp"


sim_mob::HandlerLookup::HandlerLookup() 
{
	//Register all known handlers.
	defaultHandlerMap["ALL_LOCATIONS_DATA"] = new sim_mob::AllLocationHandler();
	defaultHandlerMap["AGENTS_INFO"] = new sim_mob::AgentsInfoHandler();
	defaultHandlerMap["TIME_DATA"] = new sim_mob::NullHandler();
	defaultHandlerMap["READY_TO_RECEIVE"] = new sim_mob::NullHandler();
	defaultHandlerMap["WHOAMI"] = new sim_mob::NullHandler();
	defaultHandlerMap["CLIENT_MESSAGES_DONE"] = new sim_mob::NullHandler();
	defaultHandlerMap["REMOTE_LOG"] = new sim_mob::RemoteLogHandler();
	defaultHandlerMap["REROUTE_REQUEST"] = new sim_mob::RerouteRequestHandler();
	defaultHandlerMap["NEW_CLIENT"] = new sim_mob::NewClientHandler();

	//Help avoid common errors with old message types.
	defaultHandlerMap["MULTICAST"] = new sim_mob::ObsoleteHandler();
	defaultHandlerMap["UNICAST"] = new sim_mob::ObsoleteHandler();
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
	if (mType=="MULTICAST" || mType=="UNICAST") {
		throw std::runtime_error("Cannot register a custom handler for MULTICAST/UNICAST messages; they are obsolete (use OPAQUE_SEND/RECIEVE instead).");
	}
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



void sim_mob::OpaqueSendHandler::handle(boost::shared_ptr<ConnectionHandler> handler, const MessageConglomerate& messages, int msgNumber, Broker* broker) const
{
	//Ask the serializer for a Unicast message.
	OpaqueSendMessage ucMsg = CommsimSerializer::parseOpaqueSend(messages, msgNumber);

	throw std::runtime_error("TODO: OpaqueSend");
}


void sim_mob::OpaqueReceiveHandler::handle(boost::shared_ptr<ConnectionHandler> handler, const MessageConglomerate& messages, int msgNumber, Broker* broker) const
{
	//Ask the serializer for a Multicast message.
	OpaqueReceiveMessage mcMsg = CommsimSerializer::parseOpaqueReceive(messages, msgNumber);

	throw std::runtime_error("TODO: OpaqueReceive");
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



//handler implementation
//this handler handles the multicast requests sent by android
//the hadler does this by finding the sender's nearby agents
//please take note that in this implementation:
//1-multicast is treated same as broadcast
//2-although it is like a broadcast, simmobility will add the specific receiver
//  information while redirecting to NS3(as opposed to letting NS3 find the recipients)
void sim_mob::OpaqueSendHandler::handle(boost::shared_ptr<ConnectionHandler> handler, const MessageConglomerate& messages, int msgNumber, Broker* broker) const
{
	OpaqueSendMessage sendMsg = CommsimSerializer::parseOpaqueSend(messages, msgNumber);

	//steps:
	// 1- Find the sending agent
	// 2- Find its nearby agents
	// 3- for each agent find the client handler
	// 4- fabricate a message for each(core  is taken from the original message)
	// 5- insert messages into send buffer

	//	step-1: Find the sending agent

	//1.1: parse
	//sim_mob::comm::MsgData &data = message_->getData();
	//msg_header msg_header_;
	//if(!sim_mob::JsonParser::parseMessageHeader(data,msg_header_)) {
	//	WarnOut("ANDROID_HDL_MULTICAST::handle: message header incomplete" << std::endl);
	//	return;
	//}

	//1.2 do a double check: the sending agent is an ANDROID_EMULATOR
	//TODO: This will have to go.
	//if(sendMsg.sender_type != "ANDROID_EMULATOR") {
	//	return;
	//}

	//1.3 find the client hander first
	boost::shared_ptr<sim_mob::ClientHandler> clnHandler;
	if(!broker->getClientHandler(sendMsg.sender_id, sendMsg.sender_type, clnHandler)) {
		WarnOut( "ANDROID_HDL_MULTICAST::handle failed" << std::endl);
		return;
	}

	//Check the handler's validity.
	if(!clnHandler->isValid()) {
		Print() << "Invalid client handler record" << std::endl;
		return;
	}

	//1.4 now find the agent
	const sim_mob::Agent* original_agent = clnHandler->agent;
	if(!original_agent) {
		Print() << "Invalid agent record" << std::endl;
		return;
	}

	//step-2: get the agents around you
	std::vector<const Agent*> nearby_agents = AuraManager::instance().agentsInRect(
		Point2D((original_agent->xPos - 3500), (original_agent->yPos - 3500)),
		Point2D((original_agent->xPos + 3500), (original_agent->yPos + 3500)),
		original_agent
	);

	//TODO: We only  need to search for Agents if "broadcast" is true, but at the moment "broadcast" is tightly integrated into
	//      the algorithm below (ideally, we would (optionally) build the recipients list FIRST, and THEN attempt to send messages).
	throw std::runtime_error("TODO: If broadcast is not set, don't overwrite the Agent's list!");

	//omit the sending agent from the list of nearby_agents
	std::vector<const Agent*>::iterator it_find = std::find(nearby_agents.begin(), nearby_agents.end(), original_agent);
	if(it_find != nearby_agents.end()) {
		nearby_agents.erase(it_find);
	}

	//If there's no agents left, return.
	if(nearby_agents.size() == 0) {
		return;
	}

	//ClientList::Pair clientTypes;
	const ClientList::Type & all_clients = broker->getClientList();
	std::vector<std::string> receiveAgentIds;

	//iterate through all registered clients
	for (ClientList::Type::const_iterator clientIt=all_clients.begin(); clientIt!=all_clients.end(); clientIt++) {
	//BOOST_FOREACH(clientTypes , all_clients) {
		// filter out those client sets which are not android emulators
		//TODO: We should really filter by "isNonSpatial()" ---all agents *should* get the message if it is a multicast.
		if(clientIt->first != comm::ANDROID_EMULATOR) {
			continue;
		}

		//ClientList::ValuePair clientIds;
		boost::unordered_map<std::string , boost::shared_ptr<sim_mob::ClientHandler> >& inner = clientIt->second;

		//iterate through android emulator clients
		for (boost::unordered_map<std::string , boost::shared_ptr<sim_mob::ClientHandler> >::const_iterator it=inner.begin(); it!=inner.end(); it++) {
		//BOOST_FOREACH(clientIds , inner) {
			//get the agent associated to this client
			boost::shared_ptr<sim_mob::ClientHandler> destClientHandlr  = it->second;
			const sim_mob::Agent* agent = destClientHandlr->agent;

			//get the agent associated with the client handler and see if it is among the nearby_agents
			if(std::find(nearby_agents.begin(), nearby_agents.end(), agent) == nearby_agents.end()) {
				continue;
			}

			//step-4: fabricate a message for each(core  is taken from the original message)
				//actually, you don't need to modify any field in
				//the original jsoncpp's Json::Value message.
				//just add the recipients
			//NOTE: This part is different for ns-3 versus android-only.
			handleClient(*destClientHandlr, receiveAgentIds, *broker, sendMsg);
		}
	}

	//NOTE: This part only matters if NS3 is used.
	postPendingMessages(*broker, *original_agent, receiveAgentIds, sendMsg);
}


void sim_mob::OpaqueSendHandler::handleClient(const sim_mob::ClientHandler& clientHdlr, std::vector<std::string>& receiveAgentIds, Broker& broker, const OpaqueSendMessage& currMsg) const
{
	if (useNs3) {
		//add the agent to the list of ns3 agent recipients
		receiveAgentIds.push_back(boost::lexical_cast<std::string>(clientHdlr.agent->getId()));
	} else {
		//directly request to send
		//Note: The recipients field will not be well-formed in this case.
		std::string msg = CommsimSerializer::makeOpaqueReceive(currMsg.fromId, boost::lexical_cast<std::string>(clientHdlr.agent->getId()), currMsg.data);
		broker.insertSendBuffer(boost::shared_ptr<ClientHandler>(const_cast<ClientHandler*>(&clientHdlr)), msg);
	}
}

void sim_mob::OpaqueSendHandler::postPendingMessages(sim_mob::Broker& broker, const sim_mob::Agent& agent, const std::vector<std::string>& receiveAgentIds, const OpaqueSendMessage& currMsg) const
{
	if (useNs3) {
		//step-5: insert messages into send buffer
		//NOTE: This part only exists for ns-3+android.
		boost::shared_ptr<sim_mob::ClientHandler> ns3_clnHandler;
		broker.getClientHandler("0", "NS3_SIMULATOR", ns3_clnHandler);
		if(!receiveAgentIds.empty()) {
			//add two extra field to mark the agent ids(used in simmobility to identify agents)
			//data["SENDING_AGENT"] = agent.getId();
			//data["RECIPIENTS"] = receiveAgentIds;
			std::string msg = CommsimSerializer::makeOpaqueSend(currMsg.fromId, receiveAgentIds, currMsg.broadcast, currMsg.data);
			broker.insertSendBuffer(ns3_clnHandler, msg);
		}
	}
}


//you are going to handle something like this:
//{"MESSAGE_CAT":"APP","MESSAGE_TYPE":"MULTICAST","MULTICAST_DATA":"TVVMVElDQVNUIFN0cmluZyBmcm9tIGNsaWVudCAxMTQ=","RECEIVING_AGENT_ID":75,"SENDER":"0","SENDER_TYPE":"NS3_SIMULATOR"}
void sim_mob::OpaqueReceiveHandler::handle(boost::shared_ptr<ConnectionHandler> handler, const MessageConglomerate& messages, int msgNumber, Broker* broker) const
{
	if (!useNs3) {
		throw std::runtime_error("NS-3 handler called when ns-3 was disabled.");
	}

	OpaqueReceiveMessage recMsg = CommsimSerializer::parseOpaqueReceive(messages, msgNumber);

	//At this point, ns-3 has processed the message, so there should only be one recipient.
	//if (recMsg.recipients.size() != 1) {
	//	throw std::runtime_error("Error: recipients list should have exactly one recipient.");
	//}

	//Get the client handler for this recipient.
	boost::shared_ptr<sim_mob::ClientHandler> receiveAgentHandle;

	//find the client destination client_handler
	//boost::shared_ptr<sim_mob::ClientHandler> destination_clnHandler;
	//sim_mob::comm::MsgData& jData = message_->getData();
	//int destination_agent_id = jData["RECEIVING_AGENT_ID"].asInt();
	const ClientList::Type& all_clients = broker->getClientList();
	for (ClientList::Type::const_iterator ctypeIt=all_clients.begin(); ctypeIt!=all_clients.end(); ctypeIt++) {
	//ClientList::Pair clientTypes;
	//BOOST_FOREACH(clientTypes , all_clients) {
		// only the android emulators
		//TODO: We should really check this in a different way.
		if (ctypeIt->first != comm::ANDROID_EMULATOR) {
			continue;
		}

		//ClientList::ValuePair clientIds;
		boost::unordered_map<std::string,boost::shared_ptr<sim_mob::ClientHandler> > &inner = ctypeIt->second;
		for (boost::unordered_map<std::string,boost::shared_ptr<sim_mob::ClientHandler> >::const_iterator it=inner.begin(); it!=inner.end(); it++) {
		//BOOST_FOREACH(clientIds , inner) {
			boost::shared_ptr<sim_mob::ClientHandler> clnHandler = it->second;
			//valid agent, matching ID
			if (clnHandler->agent && boost::lexical_cast<std::string>(clnHandler->agent->getId()) == recMsg.toId) {
				receiveAgentHandle = clnHandler;
				break;
			}
		}

		//insert into sending buffer
		if (receiveAgentHandle && receiveAgentHandle->connHandle) {
			broker->insertSendBuffer(receiveAgentHandle, CommsimSerializer::makeOpaqueReceive(recMsg.fromId, recMsg.toId, recMsg.data));
		}
	}

}



