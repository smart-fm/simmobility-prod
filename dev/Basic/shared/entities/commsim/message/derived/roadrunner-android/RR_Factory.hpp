//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * RRMSGFactory.hpp
 *
 *  Created on: May 13, 2013
 *      Author: vahid
 */

#pragma once

#include <map>

#include "entities/commsim/message/Types.hpp"
#include "entities/commsim/serialization/Serialization.hpp"

#include "entities/commsim/message/derived/roadrunner-android/MULTICAST_Message.hpp"
#include "entities/commsim/message/derived/roadrunner-android/UNICAST_Message.hpp"
#include "entities/commsim/message/derived/roadrunner-android/CLIENTDONE_Message.hpp"

namespace sim_mob {
namespace roadrunner{


/***
 * This class is concerned with creating messages and message handlers based on several type strings or ids.
 * The classes RR_Factory and RR_Android_Factory are simply templatized sub-classes of this RR_FactoryBase class.
 * This was refactored to avoid duplicate code (the previous two classes were almost entirely duplicated).
 * This is NOT necessarily a clean solution; in fact, I will examine the messages and handlers to see if we
 * can share more functionality there and remove the templates entirely. ~Seth
 */
template <class MulticastHandler, class UnicastHandler, class MulticastMessage, class UnicastMessage, class ClientDoneMessage>
class RR_FactoryBase : public MessageFactory<std::vector<sim_mob::comm::MsgPtr>&, std::string&> {
	enum MessageType {
		MULTICAST = 1,
		UNICAST = 2,
		//ANNOUNCE = 3,
		//KEY_REQUEST = 4,
		//KEY_SEND = 5,
		CLIENT_MESSAGES_DONE = 6
	};

	std::map<std::string, RR_FactoryBase::MessageType> MessageMap;

	//This map is used as a cache to avoid repetitive handler creation in heap
	std::map<MessageType, boost::shared_ptr<sim_mob::Handler> > HandlerMap;

public:
	RR_FactoryBase();
	virtual ~RR_FactoryBase();

	//creates a message with correct format + assigns correct handler
	//todo improve the function to handle array of messages stored in the input string
	bool createMessage(std::string &str, std::vector<sim_mob::comm::MsgPtr>&output);

	//gets a handler either from a cache or by creating a new one
	boost::shared_ptr<sim_mob::Handler>  getHandler(MessageType);
};


///Subclass for roadrunner-only.
class RR_Factory : public RR_FactoryBase<sim_mob::roadrunner::HDL_MULTICAST, sim_mob::roadrunner::HDL_UNICAST, sim_mob::roadrunner::MSG_MULTICAST, sim_mob::roadrunner::MSG_UNICAST, sim_mob::roadrunner::MSG_CLIENTDONE> {};

} //End roadrunner namespace
} //End sim_mob namespace


///////////////////////////////////////////////////////////////////////
// Template implementation
///////////////////////////////////////////////////////////////////////


template <class MulticastHandler, class UnicastHandler, class MulticastMessage, class UnicastMessage, class ClientDoneMessage>
sim_mob::roadrunner::RR_FactoryBase<MulticastHandler, UnicastHandler, MulticastMessage, UnicastMessage, ClientDoneMessage>::RR_FactoryBase()
{
	//Doing it manually; C++1 doesn't like the boost assignment.
	MessageMap.clear();
	MessageMap["MULTICAST"] = MULTICAST;
	MessageMap["UNICAST"] = UNICAST;
	MessageMap["CLIENT_MESSAGES_DONE"] = CLIENT_MESSAGES_DONE;

	//MessageMap = boost::assign::map_list_of("MULTICAST", MULTICAST)("UNICAST", UNICAST)("CLIENT_MESSAGES_DONE",CLIENT_MESSAGES_DONE)/*("ANNOUNCE",ANNOUNCE)("KEY_REQUEST", KEY_REQUEST)("KEY_SEND",KEY_SEND)*/;
}

template <class MulticastHandler, class UnicastHandler, class MulticastMessage, class UnicastMessage, class ClientDoneMessage>
sim_mob::roadrunner::RR_FactoryBase<MulticastHandler, UnicastHandler, MulticastMessage, UnicastMessage, ClientDoneMessage>::~RR_FactoryBase()
{}



template <class MulticastHandler, class UnicastHandler, class MulticastMessage, class UnicastMessage, class ClientDoneMessage>
boost::shared_ptr<sim_mob::Handler>  sim_mob::roadrunner::RR_FactoryBase<MulticastHandler, UnicastHandler, MulticastMessage, UnicastMessage, ClientDoneMessage>::getHandler(MessageType type)
{
	boost::shared_ptr<sim_mob::Handler> handler;
	//if handler is already registered && the registered handler is not null
	typename std::map<MessageType, boost::shared_ptr<sim_mob::Handler> >::iterator it = HandlerMap.find(type);
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
			handler.reset(new MulticastHandler());
			break;
		case UNICAST:
			handler.reset(new UnicastHandler());
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


template <class MulticastHandler, class UnicastHandler, class MulticastMessage, class UnicastMessage, class ClientDoneMessage>
bool sim_mob::roadrunner::RR_FactoryBase<MulticastHandler, UnicastHandler, MulticastMessage, UnicastMessage, ClientDoneMessage>::createMessage(std::string &input, std::vector<sim_mob::comm::MsgPtr>& output)
{
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
		if (!sim_mob::JsonParser::parseMessageHeader(root[index], messageHeader)) {
			continue;
		}
		Json::Value& curr_json = root[index];
		switch (MessageMap[messageHeader.msg_type]) {
		case MULTICAST:{
			//create a message
			sim_mob::comm::MsgPtr msg(new MulticastMessage(curr_json));
			//... and then assign the handler pointer to message's member
			msg->setHandler(getHandler(MULTICAST));
			output.push_back(msg);
			break;
		}
		case UNICAST:{
			//create a message
			sim_mob::comm::MsgPtr msg(new UnicastMessage(curr_json));
			//... and then assign the handler pointer to message's member
			msg->setHandler(getHandler(UNICAST));
			output.push_back(msg);
			break;
		}
		case CLIENT_MESSAGES_DONE:{
			//create a message
			sim_mob::comm::MsgPtr msg(new ClientDoneMessage(curr_json));
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


