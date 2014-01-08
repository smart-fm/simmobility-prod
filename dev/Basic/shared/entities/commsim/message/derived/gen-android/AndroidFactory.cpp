//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "AndroidFactory.hpp"
#include "logging/Log.hpp"

using namespace sim_mob;


sim_mob::comm::AndroidFactory::AndroidFactory(bool useNs3) : useNs3(useNs3)
{
	//Doing it manually; C++1 doesn't like the boost assignment.
	MessageMap.clear();
	MessageMap["MULTICAST"] = MULTICAST;
	MessageMap["UNICAST"] = UNICAST;
	MessageMap["CLIENT_MESSAGES_DONE"] = CLIENT_MESSAGES_DONE;

	//MessageMap = boost::assign::map_list_of("MULTICAST", MULTICAST)("UNICAST", UNICAST)("CLIENT_MESSAGES_DONE",CLIENT_MESSAGES_DONE)/*("ANNOUNCE",ANNOUNCE)("KEY_REQUEST", KEY_REQUEST)("KEY_SEND",KEY_SEND)*/;
}

sim_mob::comm::AndroidFactory::~AndroidFactory()
{}



boost::shared_ptr<sim_mob::Handler>  sim_mob::comm::AndroidFactory::getHandler(MessageType type)
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
			handler.reset(new sim_mob::comm::MulticastHandler(useNs3));
			break;
		case UNICAST:
			handler.reset(new sim_mob::comm::UnicastHandler(useNs3));
			break;
		default:
			typeFound = false;
			break;
		}
		//register this baby
		if(typeFound)
		{
			HandlerMap[type] = handler;
		}
	}

	return handler;
}


bool sim_mob::comm::AndroidFactory::createMessage(const std::string &input, std::vector<sim_mob::comm::MsgPtr>& output)
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

		//Convert the message type:
		std::map<std::string, AndroidFactory::MessageType>::const_iterator it = MessageMap.find(messageHeader.msg_type);
		if (it==MessageMap.end()) {
			Warn() <<"AndroidFactory::createMessage() - Unknown message type: " <<messageHeader.msg_type <<"\n";
			return false;
		}

		const Json::Value& curr_json = root[index];
		switch (it->second) {
		case MULTICAST:{
			//create a message
			sim_mob::comm::MsgPtr msg(new MulticastMessage(curr_json, useNs3));
			//... and then assign the handler pointer to message's member
			msg->setHandler(getHandler(MULTICAST));
			output.push_back(msg);
			break;
		}
		case UNICAST:{
			//create a message
			sim_mob::comm::MsgPtr msg(new UnicastMessage(curr_json, useNs3));
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
			Warn() <<"AndroidFactory::createMessage() - Unhandled message type: " <<messageHeader.msg_type <<"\n";
			break;
		}
	}		//for loop

	return true;
}


