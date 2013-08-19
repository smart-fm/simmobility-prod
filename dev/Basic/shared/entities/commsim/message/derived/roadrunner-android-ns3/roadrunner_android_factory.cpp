/*
 * RRMSGFactory.cpp
 *
 *  Created on: May 13, 2013
 *      Author: vahid
 */

#include "roadrunner_android_factory.hpp"
#include <boost/assign/list_of.hpp> // for 'map_list_of()'
#include <json/json.h>
#include <stdexcept>
#include "logging/Log.hpp"

namespace sim_mob {
namespace rr_android_ns3 {

RR_Android_Factory::RR_Android_Factory() {
	// TODO Auto-generated constructor stub
	MessageMap = boost::assign::map_list_of("MULTICAST", MULTICAST)("UNICAST", UNICAST)("CLIENT_MESSAGES_DONE",CLIENT_MESSAGES_DONE);

}
RR_Android_Factory::~RR_Android_Factory() {}
//gets a handler either from a chche or by creating a new one
hdlr_ptr  RR_Android_Factory::getHandler(MessageType type){
	hdlr_ptr handler;
	//if handler is already registered && the registered handler is not null
	std::map<MessageType, hdlr_ptr >::iterator it = HandlerMap.find(type);
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
			handler.reset(new sim_mob::rr_android_ns3::ANDROID_HDL_MULTICAST());
			break;
		case UNICAST:
			handler.reset(new sim_mob::rr_android_ns3::ANDROID_HDL_UNICAST());
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
 bool RR_Android_Factory::createMessage(std::string &input, std::vector<msg_ptr>& output)
{
//	std::vector<msg_t> result;
//	 Print() << "inside RR_Android_Factory::createMessage" << std::endl;
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
		msg_data_t & curr_json = root[index];
		switch (MessageMap[messageHeader.msg_type]) {
		case MULTICAST:{
			//create a message
			msg_ptr msg(new sim_mob::rr_android_ns3::ANDROID_MSG_MULTICAST(curr_json));
			//... and then assign the handler pointer to message's member
			msg->setHandler(getHandler(MULTICAST));
			output.push_back(msg);
			break;
		}
		case UNICAST:{
			//create a message
			msg_ptr msg(new sim_mob::rr_android_ns3::ANDROID_MSG_UNICAST(curr_json));
			//... and then assign the handler pointer to message's member
			msg->setHandler(getHandler(UNICAST));
			output.push_back(msg);
			break;
		}
		case CLIENT_MESSAGES_DONE:{
			//create a message
			msg_ptr msg(new sim_mob::rr_android_ns3::MSG_CLIENTDONE(curr_json));
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

} /* namespace rr_android_ns3 */
} /* namespace sim_mob */
