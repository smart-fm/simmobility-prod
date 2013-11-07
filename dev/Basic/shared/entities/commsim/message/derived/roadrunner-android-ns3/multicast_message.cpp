//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * MULTICAST_Message.cpp
 *
 *  Created on: May 9, 2013
 *      Author: vahid
 */

#include "multicast_message.hpp"

#include "entities/commsim/event/subscribers/base/ClientHandler.hpp"
#include "entities/AuraManager.hpp"

using namespace sim_mob;


/***************************************************************************************************************************************************
 *****************************   ANDROID   ************************************************************************************************************
 **************************************************************************************************************************************************/

sim_mob::rr_android_ns3::ANDROID_MSG_MULTICAST::ANDROID_MSG_MULTICAST(sim_mob::comm::MsgData data_): Message(data_)
{}

Handler* sim_mob::rr_android_ns3::ANDROID_MSG_MULTICAST::newHandler()
{
	return new sim_mob::roadrunner::MulticastHandler(true);
}


/*void sim_mob::rr_android_ns3::ANDROID_HDL_MULTICAST::handleClient(const sim_mob::ClientHandler& clientHdlr, sim_mob::comm::MsgData& recipientsList, Broker& broker, sim_mob::comm::MsgData& data)
{
	recipientsList.append(clientHdlr.agent->getId());
}

void sim_mob::rr_android_ns3::ANDROID_HDL_MULTICAST::postPendingMessages(sim_mob::Broker& broker, const sim_mob::Agent& agent, const sim_mob::comm::MsgData& recipientsList, sim_mob::comm::MsgData& data)
{
	//step-5: insert messages into send buffer
	//NOTE: This part only exists for ns-3+android.
	boost::shared_ptr<sim_mob::ClientHandler> ns3_clnHandler;
	broker.getClientHandler("0", "NS3_SIMULATOR", ns3_clnHandler);
	if(recipientsList.size()>0) {
		//add two extra field to mark the agent ids(used in simmobility to identify agents)
		data["SENDING_AGENT"] = agent.getId();
		data["RECIPIENTS"] = recipientsList;
		broker.insertSendBuffer(ns3_clnHandler->cnnHandler,data);
	}
}*/



/***************************************************************************************************************************************************
 * ****************************   NS3   ************************************************************************************************************
 * ************************************************************************************************************************************************
 */
sim_mob::rr_android_ns3::NS3_MSG_MULTICAST::NS3_MSG_MULTICAST(sim_mob::comm::MsgData data_): Message(data_)
{}

Handler* sim_mob::rr_android_ns3::NS3_MSG_MULTICAST::newHandler()
{
	return new NS3_HDL_MULTICAST();
}


//you are going to handle something like this:
//{"MESSAGE_CAT":"APP","MESSAGE_TYPE":"MULTICAST","MULTICAST_DATA":"TVVMVElDQVNUIFN0cmluZyBmcm9tIGNsaWVudCAxMTQ=","RECEIVING_AGENT_ID":75,"SENDER":"0","SENDER_TYPE":"NS3_SIMULATOR"}
void sim_mob::rr_android_ns3::NS3_HDL_MULTICAST::handle(sim_mob::comm::MsgPtr message_, Broker* broker) {
	//find the client destination client_handler
	boost::shared_ptr<sim_mob::ClientHandler> destination_clnHandler;
	sim_mob::comm::MsgData& jData = message_->getData();
	int destination_agent_id = jData["RECEIVING_AGENT_ID"].asInt();
	ClientList::type & all_clients = broker->getClientList();
	ClientList::pair clientTypes;
	BOOST_FOREACH(clientTypes , all_clients) {
		// only the android emulators
		if (clientTypes.first != ConfigParams::ANDROID_EMULATOR) {
			continue;
		}

		ClientList::IdPair clientIds;
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
			destination_clnHandler = clnHandler;
			//no need to search again once the first -and only- match is found
			break;
		}

		//insert into sending buffer
		if (destination_clnHandler && destination_clnHandler->cnnHandler) {
			broker->insertSendBuffer(destination_clnHandler->cnnHandler, jData);
		}
	}

}	//handle()




