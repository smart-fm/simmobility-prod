/*
 * MULTICAST_Message.cpp
 *
 *  Created on: May 9, 2013
 *      Author: vahid
 */

#include "android_multicast_message.hpp"

#include "entities/commsim/event/subscribers/base/ClientHandler.hpp"
#include "entities/AuraManager.hpp"

namespace sim_mob {
class Handler;

namespace rr_android_ns3
{
class ANDROID_HDL_MULTICAST;
ANDROID_MSG_MULTICAST::ANDROID_MSG_MULTICAST(msg_data_t data_): Message(data_)
{

}
Handler * ANDROID_MSG_MULTICAST::newHandler()
{
	return new ANDROID_HDL_MULTICAST();
}


//handler implementation
//this handler handles the multicast requests sent by android
//the hadler does this by finding the sender's nearby agents
//please take note that in this implementation:
//1-multicast is treated same as broadcast
//2-although it is like a broadcast, simmobility will add the specific receiver
//information while redirecting to NS3(as opposed to letting NS3 find the recipients)
void ANDROID_HDL_MULTICAST::handle(msg_ptr message_,Broker* broker){
//steps:
	/*
	 * 1- Find the sending agent
	 * 2- Find its nearby agents
	 * 3- for each agent find the client handler
	 * 4- fabricate a message for each(core  is taken from the original message)
	 * 5- insert messages into send buffer
	 */

//	step-1: Find the sending agent

	//1.1: parse
	Json::Value &jdata = message_->getData();
	msg_header msg_header_;
	if(!sim_mob::JsonParser::parseMessageHeader(jdata,msg_header_))
	{
		WarnOut("ANDROID_HDL_MULTICAST::handle: message header incomplete" << std::endl);
		return;
	}

	//1.2 do a double check: the sending agent is an ANDROID_EMULATOR
	if(msg_header_.sender_type != "ANDROID_EMULATOR")
	{
		return;
	}

	//1.3 find the client hander first
	std::string sender_id(msg_header_.sender_id) ; //easy read
	std::string sender_type(msg_header_.sender_type); //easy read
	ConfigParams::ClientType clientType;
	boost::shared_ptr<sim_mob::ClientHandler> clnHandler;
	if(!broker->getClientHandler(sender_id,sender_type,clnHandler))
	{
		WarnOut( "ANDROID_HDL_MULTICAST::handle failed" << std::endl);
		Print() << "ANDROID_HDL_MULTICAST::handle failed" << std::endl;
		return;
	}
	if(!clnHandler->isValid())
	{
		Print() << "Invalid ns3 client handler record" << std::endl;
		return;
	}

	//1.4 now find the agent
	const sim_mob::Agent * sending_agent = clnHandler->agent;

	//step-2: get the agents around you
	std::vector<const Agent*> nearby_agents = AuraManager::instance().agentsInRect(

				Point2D(

						(sending_agent->xPos - 3500),
						(sending_agent->yPos - 3500)
						)

						,

				Point2D(
						(sending_agent->xPos + 3500),
						(sending_agent->yPos + 3500)
						)

						);
	//omit the sending agent from the list of nearby_agents
	std::vector<const Agent*>::iterator it_find = std::find(nearby_agents.begin(), nearby_agents.end(), sending_agent);
	if(it_find != nearby_agents.end())
	{
		nearby_agents.erase(it_find);
	}
	else
	{
		Print()<< "Error: nearby agents doesn't even include the central agent" << std::endl;
		return;
	}
	if(nearby_agents.size() == 0)
	{
		Print() << "Debug: No agent qualified(out of " << nearby_agents.size() << ")" << std::endl;
		return;
	}

	//step-3: for each agent find the client handler
	boost::shared_ptr<sim_mob::ClientHandler> ns3_clnHandler;
	broker->getClientHandler("0", "NS3_SIMULATOR", ns3_clnHandler);
	ClientList::pair clientTypes;
	ClientList::type & all_clients = broker->getClientList();
	Json::Value recipients;
	BOOST_FOREACH(clientTypes , all_clients)
	{
		// only the android emulators
		if(clientTypes.first != ConfigParams::ANDROID_EMULATOR)
		{
			continue;
		}

		ClientList::IdPair clientIds;
		boost::unordered_map<std::string , boost::shared_ptr<sim_mob::ClientHandler> > &inner = clientTypes.second;
		BOOST_FOREACH(clientIds , inner)
		{
			boost::shared_ptr<sim_mob::ClientHandler> destination_agent_clnHandler  = clientIds.second;
			//get the agent associated with the client handler and see if it is among the nearby_agents
			if(std::find(nearby_agents.begin(), nearby_agents.end(), destination_agent_clnHandler->agent) == nearby_agents.end())
			{
//				Print() << "Debug: agent not found" << std::endl;
				continue;
			}
			else
			{
//				Print() << "Debug: a destination agent found" << std::endl;
			}
			//step-4: fabricate a message for each(core  is taken from the original message)
				//actually, you don't need to modify any field in
				//the original jsoncpp's Json::Value message.
				//just add the recipients
//			Print() << "Debug: agent found, inserting" << std::endl;
			recipients.append(destination_agent_clnHandler->agent->getId());
		}//inner loop : BOOST_FOREACH(clientIds , inner)
	}//outer loop : BOOST_FOREACH(clientTypes , clients)
	//step-5: insert messages into send buffer
	if(recipients.size())
	{
		//add two extra field to mark the agent ids(used in simmobility to identify agents)
		jdata["SENDING_AGENT"] = sending_agent->getId();
		jdata["RECIPIENTS"] = recipients;
		broker->insertSendBuffer(ns3_clnHandler->cnnHandler,jdata);
	}
}//handle()

}/* namespace rr_android_ns3 */
} /* namespace sim_mob */



