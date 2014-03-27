//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "MulticastHandler.hpp"

#include <boost/unordered_map.hpp>

#include "entities/commsim/event/subscribers/base/ClientHandler.hpp"
#include "entities/AuraManager.hpp"

#include "entities/commsim/message/Messages.hpp"
#include "entities/commsim/serialization/CommsimSerializer.hpp"

using namespace sim_mob;
/*

sim_mob::roadrunner::MulticastHandler::MulticastHandler(bool useNs3) : useNs3(useNs3)
{
}



//handler implementation
//this handler handles the multicast requests sent by android
//the hadler does this by finding the sender's nearby agents
//please take note that in this implementation:
//1-multicast is treated same as broadcast
//2-although it is like a broadcast, simmobility will add the specific receiver
//  information while redirecting to NS3(as opposed to letting NS3 find the recipients)
void sim_mob::roadrunner::MulticastHandler::handle(boost::shared_ptr<ConnectionHandler> handler, const MessageConglomerate& messages, int msgNumber, Broker* broker) const
{
	MulticastMessage mcMsg = CommsimSerializer::parseMulticast(messages, msgNumber);

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
	if(mcMsg.sender_type != "ANDROID_EMULATOR") {
		return;
	}

	//1.3 find the client hander first
	boost::shared_ptr<sim_mob::ClientHandler> clnHandler;
	if(!broker->getClientHandler(mcMsg.sender_id, mcMsg.sender_type, clnHandler)) {
		WarnOut( "ANDROID_HDL_MULTICAST::handle failed" << std::endl);
		return;
	}

	//Check the handler's validity.
	if(!clnHandler->isValid()) {
		Print() << "Invalid client handler record" << std::endl;
		return;
	}

	//1.4 now find the agent
	const sim_mob::Agent * original_agent = clnHandler->agent;
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
	std::vector<unsigned int> receiveAgentIds;

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
			handleClient(*destClientHandlr, receiveAgentIds, *broker, mcMsg);
		}
	}

	//NOTE: This part only matters if NS3 is used.
	postPendingMessages(*broker, *original_agent, receiveAgentIds, mcMsg);
}


void sim_mob::roadrunner::MulticastHandler::handleClient(const sim_mob::ClientHandler& clientHdlr, std::vector<unsigned int>& receiveAgentIds, Broker& broker, MulticastMessage& currMsg)
{
	if (useNs3) {
		//add the agent to the list of ns3 agent recipients
		receiveAgentIds.push_back(clientHdlr.agent->getId());
	} else {
		//directly request to send
		//Note: The recipients field will not be well-formed in this case.
		std::string msg = CommsimSerializer::makeMulticast(clientHdlr.agent->getId(), receiveAgentIds, currMsg.msgData);
		broker.insertSendBuffer(boost::shared_ptr<ClientHandler>(const_cast<ClientHandler*>(&clientHdlr)), msg);
	}
}

void sim_mob::roadrunner::MulticastHandler::postPendingMessages(sim_mob::Broker& broker, const sim_mob::Agent& agent, const std::vector<unsigned int>& receiveAgentIds, MulticastMessage& currMsg)
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
			std::string msg = CommsimSerializer::makeMulticast(agent.getId(), receiveAgentIds, currMsg.msgData);
			broker.insertSendBuffer(ns3_clnHandler, msg);
		}
	}
}

*/
