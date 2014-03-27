//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * MULTICAST_Message.cpp
 *
 *  Created on: May 9, 2013
 *      Author: vahid
 */

#include "MulticastMessage.hpp"

#include <boost/lexical_cast.hpp>
#include <boost/unordered_map.hpp>

#include "entities/commsim/event/subscribers/base/ClientHandler.hpp"
#include "entities/AuraManager.hpp"

using namespace sim_mob;



/***************************************************************************************************************************************************
 * ****************************   NS3   ************************************************************************************************************
 * ************************************************************************************************************************************************
 */
/*sim_mob::rr_android_ns3::NS3_MSG_MULTICAST::NS3_MSG_MULTICAST(const sim_mob::comm::MsgData& data_): Message(data_)
{}

Handler* sim_mob::rr_android_ns3::NS3_MSG_MULTICAST::newHandler()
{
	return new NS3_HDL_MULTICAST();
}*/

/*

//you are going to handle something like this:
//{"MESSAGE_CAT":"APP","MESSAGE_TYPE":"MULTICAST","MULTICAST_DATA":"TVVMVElDQVNUIFN0cmluZyBmcm9tIGNsaWVudCAxMTQ=","RECEIVING_AGENT_ID":75,"SENDER":"0","SENDER_TYPE":"NS3_SIMULATOR"}
void sim_mob::rr_android_ns3::NS3_HDL_MULTICAST::handle(boost::shared_ptr<ConnectionHandler> handler, const MessageConglomerate& messages, int msgNumber, Broker* broker) const
{
	if (!useNs3) {
		throw std::runtime_error("NS-3 handler called when ns-3 was disabled.");
	}

	MulticastMessage mcMsg = CommsimSerializer::parseMulticast(messages, msgNumber);

	//At this point, ns-3 has processed the message, so there should only be one recipient.
	if (mcMsg.recipients.size() != 1) {
		throw std::runtime_error("Error: recipients list should have exactly one recipient.");
	}

	//Get the client handler for this recipient.
	unsigned int receiveAgentId = mcMsg.recipients.front();
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
			if (clnHandler->agent && clnHandler->agent->getId() == receiveAgentId) {
				receiveAgentHandle = clnHandler;
				break;
			}
		}

		//insert into sending buffer
		if (receiveAgentHandle && receiveAgentHandle->connHandle) {
			broker->insertSendBuffer(receiveAgentHandle, CommsimSerializer::makeMulticast(boost::lexical_cast<unsigned int>(mcMsg.sender_id), mcMsg.recipients, mcMsg.msgData));
		}
	}

}

*/

