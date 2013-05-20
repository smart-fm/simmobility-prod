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
#include "Serialization.hpp"
#include "logging/Log.hpp"

namespace sim_mob {
namespace roadrunner {

RR_Factory::RR_Factory() {
	// TODO Auto-generated constructor stub
	MessageMap = boost::assign::map_list_of("ANNOUNCE", ANNOUNCE)("KEY_REQUEST", KEY_REQUEST)("KEY_SEND",KEY_SEND);

}

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
		handler.reset(new sim_mob::roadrunner::HDL_ANNOUNCE());
		HandlerMap[type] = handler;
	}

	return handler;
}

//creates a message with correct format + assigns correct handler
msg_ptr RR_Factory::createMessage(std::string message)
{
	std::string type, data;
	Json::Value root_;
	sim_mob::JsonParser::getMessageTypeAndData(message, type, data, root_);
	boost::shared_ptr<sim_mob::roadrunner::RoadrunnerMessage> msg;
	switch(MessageMap[type])
	{
	case ANNOUNCE:
		//{"messageType":"ANNOUNCE", "ANNOUNCE" : {"Sender":"clientIdxxx", "x":"346378" , "y":"734689237", "OfferingTokens":["A", "B", "C"]}}
		//no need to parse the message much:
		//just update its x,y location, then forward it to nearby agents

		//create a message
		msg.reset(new sim_mob::roadrunner::MSG_ANNOUNCE(data));


		//... and then assign the handler pointer to message's member
		msg->setHandler(getHandler(ANNOUNCE));

		return msg;
//	case KEY_REQUEST:
//		//data is : {"messageType":"KEY_REQUEST", "KEY_REQUEST" : {"Sender":"clientIdxxx", "Receiver" : "clientIdyyy", "RequestingTokens":["A", "B", "C"]}}
//		//just extract the receiver and forward the string to it without modifications
//		handleKEY_REQUEST(message);
//		break;
//	case KEY_SEND:
//		break;
	default:
		WarnOut("RR_Factory::createMessage() - Unhandled message type.");
	}

	throw std::runtime_error("Message not handled.");
}

} /* namespace roadrunner */
} /* namespace sim_mob */
