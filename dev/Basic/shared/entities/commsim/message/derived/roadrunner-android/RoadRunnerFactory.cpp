//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "RoadRunnerFactory.hpp"
#include "logging/Log.hpp"
#include "entities/commsim/message/derived/roadrunner-android/RemoteLogMessage.hpp"
#include "entities/commsim/message/derived/roadrunner-android/RemoteLogHandler.hpp"
#include "entities/commsim/message/derived/roadrunner-android/RerouteRequestMessage.hpp"
#include "entities/commsim/message/derived/roadrunner-android/RerouteRequestHandler.hpp"
#include "entities/commsim/message/derived/roadrunner-android/NewClientMessage.hpp"
#include "entities/commsim/message/derived/roadrunner-android/NewClientHandler.hpp"
#include "entities/commsim/message/base/WhoAmIMessage.hpp"

using namespace sim_mob;


sim_mob::roadrunner::RoadRunnerFactory::RoadRunnerFactory(bool useNs3) : useNs3(useNs3)
{
	//Doing it manually; C++1 doesn't like the boost assignment.
	MessageMap.clear();
	MessageMap["MULTICAST"] = MULTICAST;
	MessageMap["UNICAST"] = UNICAST;
	MessageMap["CLIENT_MESSAGES_DONE"] = CLIENT_MESSAGES_DONE;
	MessageMap["REMOTE_LOG"] = REMOTE_LOG;
	MessageMap["REROUTE_REQUEST"] = REROUTE_REQUEST;
	MessageMap["NEW_CLIENT"] = NEW_CLIENT;
	MessageMap["WHOAMI"] = WHOAMI;

	//This has to be done at creation time.
	HandlerMap[MULTICAST] = boost::shared_ptr<sim_mob::Handler>(new sim_mob::roadrunner::MulticastHandler(useNs3));
	HandlerMap[UNICAST] = boost::shared_ptr<sim_mob::Handler>(new sim_mob::roadrunner::UnicastHandler(useNs3));
	HandlerMap[REMOTE_LOG] = boost::shared_ptr<sim_mob::Handler>(new sim_mob::roadrunner::RemoteLogHandler());
	HandlerMap[REROUTE_REQUEST] = boost::shared_ptr<sim_mob::Handler>(new sim_mob::roadrunner::RerouteRequestHandler());
	HandlerMap[NEW_CLIENT] = boost::shared_ptr<sim_mob::Handler>(new sim_mob::roadrunner::NewClientHandler());
}

sim_mob::roadrunner::RoadRunnerFactory::~RoadRunnerFactory()
{}



boost::shared_ptr<sim_mob::Handler>  sim_mob::roadrunner::RoadRunnerFactory::getHandler(MessageType type) const
{
	boost::shared_ptr<sim_mob::Handler> handler;
	//if handler is already registered && the registered handler is not null
	typename std::map<MessageType, boost::shared_ptr<sim_mob::Handler> >::const_iterator it = HandlerMap.find(type);
	if((it != HandlerMap.end())&&((*it).second!= 0)) {
		//get the handler ...
		handler = (*it).second;
	} else {
 		throw std::runtime_error("No handler entry found; can't modify HandlerMap at runtime.");

		//else, create a cache entry ...
		/*switch(type)
		{
		case MULTICAST:
			handler.reset(new sim_mob::roadrunner::MulticastHandler(useNs3));
			break;
		case UNICAST:
			handler.reset(new sim_mob::roadrunner::UnicastHandler(useNs3));
			break;
		case REMOTE_LOG:
			handler.reset(new sim_mob::roadrunner::RemoteLogHandler());
			break;
		case REROUTE_REQUEST:
			handler.reset(new sim_mob::roadrunner::RerouteRequestHandler());
			break;
		case NEW_CLIENT:
			handler.reset(new sim_mob::roadrunner::NewClientHandler());
			break;
		default:
			Warn() <<"Unknown handler for given handler type: " <<type <<"\n";
			throw std::runtime_error("Unknown handler.");
		}
		//register this baby
		HandlerMap[type] = handler;*/
	}

	return handler;
}


void sim_mob::roadrunner::RoadRunnerFactory::createMessage(const std::string &input, std::vector<sim_mob::comm::MsgPtr>& output) const
{
	Json::Value root;
	sim_mob::pckt_header packetHeader;
	if(!sim_mob::JsonParser::parsePacketHeader(input, packetHeader, root)) {
		Warn() <<"RoadRunnerFactory::createMessage() cannot parse packet header.\n";
		return;
	}
	if(!sim_mob::JsonParser::getPacketMessages(input,root)) {
		Warn() <<"RoadRunnerFactory::createMessage() cannot parse packet message.\n";
		return;
	}
	for (int index = 0; index < root.size(); index++) {
		msg_header messageHeader;
		if (!sim_mob::JsonParser::parseMessageHeader(root[index], messageHeader)) {
			Warn() <<"RoadRunnerFactory::createMessage() cannot parse message header.\n";
			continue;
		}

		//Convert the message type:
		std::map<std::string, RoadRunnerFactory::MessageType>::const_iterator it = MessageMap.find(messageHeader.msg_type);
		if (it==MessageMap.end()) {
			Warn() <<"RoadRunnerFactory::createMessage() - Unknown message type: " <<messageHeader.msg_type <<"\n";
			continue;
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
		case REMOTE_LOG: {
			sim_mob::comm::MsgPtr msg(new sim_mob::roadrunner::RemoteLogMessage(curr_json));
			msg->setHandler(getHandler(REMOTE_LOG));
			output.push_back(msg);
			break;
		}
		case REROUTE_REQUEST: {
			sim_mob::comm::MsgPtr msg(new sim_mob::roadrunner::RerouteRequestMessage(curr_json));
			msg->setHandler(getHandler(REROUTE_REQUEST));
			output.push_back(msg);
			break;
		}
		case NEW_CLIENT: {
			sim_mob::comm::MsgPtr msg(new sim_mob::roadrunner::NewClientMessage(curr_json));
			msg->setHandler(getHandler(NEW_CLIENT));
			output.push_back(msg);
			break;
		}
		case WHOAMI: {
			//No handler for this message type.
			sim_mob::comm::MsgPtr msg(new sim_mob::WhoAmIMessage(curr_json));
			output.push_back(msg);
			break;
		}

		default:
			Warn() <<"RoadRunnerFactory::createMessage() - Unhandled message type: " <<messageHeader.msg_type <<"\n";
			break;
		}
	}		//for loop
}


