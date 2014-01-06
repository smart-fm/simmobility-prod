//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * UNICAST_Message.cpp
 *
 *  Created on: May 9, 2013
 *      Author: vahid
 */

#include "UnicastMessage.hpp"

#include "entities/commsim/event/subscribers/base/ClientHandler.hpp"

using namespace sim_mob;


sim_mob::rr_android_ns3::NS3_MSG_UNICAST::NS3_MSG_UNICAST(const sim_mob::comm::MsgData& data_): Message(data_)
{}

Handler* sim_mob::rr_android_ns3::NS3_MSG_UNICAST::newHandler()
{
	return new NS3_HDL_UNICAST();
}

//handler implementation
void sim_mob::rr_android_ns3::NS3_HDL_UNICAST::handle(sim_mob::comm::MsgPtr message_,Broker* broker)
{
	//find the client destination client_handler
	boost::shared_ptr<sim_mob::ClientHandler> destination_clnHandler;
	sim_mob::comm::MsgData& jData = message_->getData();
	int destination_agent_id = jData["RECEIVING_AGENT_ID"].asInt();
	ClientList::Type & all_clients = broker->getClientList();
	ClientList::Pair clientTypes;
	BOOST_FOREACH(clientTypes , all_clients) {
		// only the android emulators
		if (clientTypes.first != comm::ANDROID_EMULATOR) {
			continue;
		}

		ClientList::ValuePair clientIds;
		boost::unordered_map<std::string,
				boost::shared_ptr<sim_mob::ClientHandler> > &inner =
				clientTypes.second;
		if (inner.size() == 0) {
		} else {
		}
		BOOST_FOREACH(clientIds , inner) {
			boost::shared_ptr<sim_mob::ClientHandler> clnHandler =
					clientIds.second;
			const Agent* agent;
			//valid agent
			if ((agent = clnHandler->agent) == 0) {
				continue;
			}
			//match agent id
			if (agent->getId() != destination_agent_id) {
				continue;
			}
			//destination client handler(for android)
			destination_clnHandler = clnHandler;
			//no need to search again once the first -and only- match is found
			break;
		}

		//insert into sending buffer
		if (destination_clnHandler && destination_clnHandler->cnnHandler) {
			broker->insertSendBuffer(destination_clnHandler->cnnHandler, jData);
		}
	}
}



