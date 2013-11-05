//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * RRMSGFactory.cpp
 *
 *  Created on: May 13, 2013
 *      Author: vahid
 */

#include "RR_Factory.hpp"
#include <boost/assign/list_of.hpp> // for 'map_list_of()'
#include <json/json.h>
#include <stdexcept>
#include "logging/Log.hpp"

namespace sim_mob {
namespace roadrunner {

RR_Factory::RR_Factory() {
	//Doing it manually; C++1 doesn't like the boost assignment.
	MessageMap.clear();
	MessageMap["MULTICAST"] = MULTICAST;
	MessageMap["UNICAST"] = UNICAST;
	MessageMap["CLIENT_MESSAGES_DONE"] = CLIENT_MESSAGES_DONE;

	//MessageMap = boost::assign::map_list_of("MULTICAST", MULTICAST)("UNICAST", UNICAST)("CLIENT_MESSAGES_DONE",CLIENT_MESSAGES_DONE)/*("ANNOUNCE",ANNOUNCE)("KEY_REQUEST", KEY_REQUEST)("KEY_SEND",KEY_SEND)*/;
}
RR_Factory::~RR_Factory() {}
//gets a handler either from a chche or by creating a new one
boost::shared_ptr<sim_mob::Handler>  RR_Factory::getHandler(MessageType type){
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
			handler.reset(new sim_mob::roadrunner::HDL_MULTICAST());
			break;
		case UNICAST:
			handler.reset(new sim_mob::roadrunner::HDL_UNICAST());
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
 bool RR_Factory::createMessage(std::string &input, std::vector<msg_ptr>& output)
{
//	std::vector<msg_t> result;
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
		msg_header messageHeader;
//		std::string  msgStr;// =  /*const_cast<std::string&>*/(root[index].asString());
		if (!sim_mob::JsonParser::parseMessageHeader(root[index], messageHeader)) {
			continue;
		}
		Json::Value& curr_json = root[index];
		switch (MessageMap[messageHeader.msg_type]) {
		case MULTICAST:{
			//create a message
			msg_ptr msg(new sim_mob::roadrunner::MSG_MULTICAST(curr_json));
			//... and then assign the handler pointer to message's member
			msg->setHandler(getHandler(MULTICAST));
			output.push_back(msg);
			break;
		}
		case UNICAST:{
			//create a message
			msg_ptr msg(new sim_mob::roadrunner::MSG_UNICAST(curr_json));
			//... and then assign the handler pointer to message's member
			msg->setHandler(getHandler(UNICAST));
			output.push_back(msg);
			break;
		}
		case CLIENT_MESSAGES_DONE:{
			//create a message
			msg_ptr msg(new sim_mob::roadrunner::MSG_CLIENTDONE(curr_json));
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

} /* namespace roadrunner */
} /* namespace sim_mob */
