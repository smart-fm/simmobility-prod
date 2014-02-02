//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "BasicMessageFactory.hpp"

#include <json/json.h>

#include "entities/commsim/message/base/WhoAmIMessage.hpp"
#include "logging/Log.hpp"


using namespace sim_mob;


sim_mob::BasicMessageFactory::~BasicMessageFactory()
{}


void sim_mob::BasicMessageFactory::createMessage(const std::string &input, std::vector<sim_mob::comm::MsgPtr>& output) const
{
	Json::Value root;
	sim_mob::pckt_header packetHeader;
	if(!sim_mob::JsonParser::parsePacketHeader(input, packetHeader, root)) {
		return;
	}
	if(!sim_mob::JsonParser::getPacketMessages(input,root)) {
		return;
	}

	for (int index = 0; index<root.size(); index++) {
		msg_header messageHeader;
		if (!sim_mob::JsonParser::parseMessageHeader(root[index], messageHeader)) {
			continue;
		}

		//Convert the message type:
		if (messageHeader.msg_type != "WHOAMI") {
			Warn() <<"BasicMessageFactory::createMessage() - Unknown message type: " <<messageHeader.msg_type <<"\n";
			continue;
		}

		//Create the message
		const Json::Value& curr_json = root[index];
		sim_mob::comm::MsgPtr msg(new WhoAmIMessage(curr_json));
		//... and then assign the handler pointer to message's member
		output.push_back(msg);
	}
}


