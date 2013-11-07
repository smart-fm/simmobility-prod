//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * RRMSGFactory.cpp
 *
 *  Created on: May 13, 2013
 *      Author: vahid
 */

#include "roadrunner_ns3_factory.hpp"

#include <boost/assign/list_of.hpp>
#include <json/json.h>
#include <stdexcept>

#include "logging/Log.hpp"

using namespace sim_mob;


sim_mob::rr_android_ns3::RR_NS3_Factory::RR_NS3_Factory() {
	MessageMap = boost::assign::map_list_of("MULTICAST", MULTICAST)("UNICAST", UNICAST)("CLIENT_MESSAGES_DONE",CLIENT_MESSAGES_DONE);
}

sim_mob::rr_android_ns3::RR_NS3_Factory::~RR_NS3_Factory()
{}

//gets a handler either from a chche or by creating a new one
boost::shared_ptr<sim_mob::Handler>  sim_mob::rr_android_ns3::RR_NS3_Factory::getHandler(MessageType type)
{
	boost::shared_ptr<sim_mob::Handler> handler;
 	//if handler is already registered && the registered handler is not null
 	std::map<MessageType, boost::shared_ptr<sim_mob::Handler> >::iterator it = HandlerMap.find(type);
 	if((it != HandlerMap.end())&&((*it).second!= 0))
 	{
 		//get the handler ...
 		handler = (*it).second;
 	}
 	else
 	{
 		//else, create a cache entry ...
 		bool typeFound = true;
 		switch(type)
 		{
 		case MULTICAST:
 			handler.reset(new sim_mob::rr_android_ns3::NS3_HDL_MULTICAST());
 			break;
 		case UNICAST:
 			handler.reset(new sim_mob::rr_android_ns3::NS3_HDL_UNICAST());
 			break;
 		default:
 			typeFound = false;
 		}
 		//register this baby
 		if(typeFound)
 		{
 			HandlerMap[type] = handler;
 		}
 	}

 	return handler;
 }


//creates a message with correct format + assigns correct handler
//todo improve the function to handle array of messages stored in the input string
bool sim_mob::rr_android_ns3::RR_NS3_Factory::createMessage(std::string &input, std::vector<sim_mob::comm::MsgPtr>& output)
{
	//	std::vector<msg_t> result;
	//	 Print() << "inside RR_NS3_Factory::createMessage" << std::endl;
 	std::string type, data;
 	Json::Value root;
 	sim_mob::pckt_header packetHeader;
 	if(!sim_mob::JsonParser::parsePacketHeader(input, packetHeader, root))
 	{
 		return false;
 	}
 	if(!sim_mob::JsonParser::getPacketMessages(input,root))
 	{
 		return false;
 	}
 	for (int index = 0; index < root.size(); index++) {

 		Json::FastWriter w;
 		msg_header messageHeader;
 //		std::string  msgStr;// =  /*const_cast<std::string&>*/(root[index].asString());
 		if (!sim_mob::JsonParser::parseMessageHeader(root[index], messageHeader)) {
 			continue;
 		}
 		Json::Value& curr_json = root[index];
 		switch (MessageMap[messageHeader.msg_type]) {
 		case MULTICAST:{
 			//create a message
 			sim_mob::comm::MsgPtr msg(new sim_mob::rr_android_ns3::NS3_MSG_MULTICAST(curr_json));
 			//... and then assign the handler pointer to message's member
 			msg->setHandler(getHandler(MULTICAST));
 			output.push_back(msg);
 			break;
 		}
 		case UNICAST:{
 			//create a message
 			sim_mob::comm::MsgPtr msg(new sim_mob::rr_android_ns3::NS3_MSG_UNICAST(curr_json));
 			//... and then assign the handler pointer to message's member
 			msg->setHandler(getHandler(UNICAST));
 			output.push_back(msg);
 			break;
 		}
 		case CLIENT_MESSAGES_DONE:{
 			//create a message
 			sim_mob::comm::MsgPtr msg(new sim_mob::roadrunner::ClientDoneMessage(curr_json));
 			//... and then assign the handler pointer to message's member
 //			msg->setHandler(getHandler()); no handler!
 			output.push_back(msg);
 			break;
 		}


 		default:
 			WarnOut("RR_Factory::createMessage() - Unhandled message type.");
 		}
 	}		//for loop

 	return true;
}

