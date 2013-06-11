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
//#include "entities/commsim/communicator/service/services.hpp"
#include "logging/Log.hpp"

namespace sim_mob {
namespace roadrunner {

RR_Factory::RR_Factory() {
	// TODO Auto-generated constructor stub
	MessageMap = boost::assign::map_list_of("MULTICAST_ANNOUNCE", MULTICAST_ANNOUNCE)("KEY_REQUEST", KEY_REQUEST)("KEY_SEND",KEY_SEND);

}
RR_Factory::~RR_Factory() {}
//gets a handler either from a chche or by creating a new one
hdlr_ptr  RR_Factory::getHandler(MessageType type){
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
		case MULTICAST_ANNOUNCE:
			handler.reset(new sim_mob::roadrunner::HDL_ANNOUNCE());
			break;
		case KEY_REQUEST:
			handler.reset(new sim_mob::roadrunner::HDL_KEY_REQUEST);
			break;
		case KEY_SEND:
			handler.reset(new sim_mob::roadrunner::HDL_KEY_SEND);
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
	Print() << "nof_messages = " << root.size() << std::endl;
	for (int index = 0; index < root.size(); index++) {
		Print() << "index " << index << std::endl;
		msg_header messageHeader;
//		std::string  msgStr;// =  /*const_cast<std::string&>*/(root[index].asString());
		if (!sim_mob::JsonParser::parseMessageHeader(root[index], messageHeader)) {
			continue;
		}
		msg_data_t & curr_json = root[index];
		Print() << "switch case(" << messageHeader.msg_type << ")" << std::endl;
		switch (MessageMap[messageHeader.msg_type]) {
		case MULTICAST_ANNOUNCE:{
			//{"messageType":"ANNOUNCE", "ANNOUNCE" : {"Sender":"clientIdxxx", "x":"346378" , "y":"734689237", "OfferingTokens":["A", "B", "C"]}}
			//no need to parse the message much:
			//just update its x,y location, then forward it to nearby agents

			//create a message
			msg_ptr msg(new sim_mob::roadrunner::MSG_ANNOUNCE(curr_json));
			//... and then assign the handler pointer to message's member
			msg->setHandler(getHandler(MULTICAST_ANNOUNCE));
			output.push_back(msg);
			break;
		}

		case KEY_REQUEST:{
			//data is : {"messageType":"KEY_REQUEST", "KEY_REQUEST" : {"Sender":"clientIdxxx", "Receiver" : "clientIdyyy", "RequestingTokens":["A", "B", "C"]}}
			//just extract the receiver and forward the string to it without modifications

			//create a message
			msg_ptr msg(new sim_mob::roadrunner::MSG_KEY_REQUEST(curr_json));
			//... and then assign the handler pointer to message's member
			msg->setHandler(getHandler(KEY_REQUEST));
			output.push_back(msg);
			break;
		}
		case KEY_SEND:{
			//create a message
			msg_ptr msg(new sim_mob::roadrunner::MSG_KEY_SEND(curr_json));
			//... and then assign the handler pointer to message's member
			msg->setHandler(getHandler(KEY_SEND));
			output.push_back(msg);
			break;
		}
		default:
			WarnOut("RR_Factory::createMessage() - Unhandled message type.");
//		//todo replace properly
//		std::cout<<"RR_Factory::createMessage() - Unhandled message type." << std::endl;
		}
	}		//for loop

	return true;
}

} /* namespace roadrunner */
} /* namespace sim_mob */
