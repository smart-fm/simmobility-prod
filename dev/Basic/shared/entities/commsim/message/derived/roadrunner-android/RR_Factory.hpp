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

class RR_Factory : public MessageFactory<std::vector<sim_mob::comm::MsgPtr>&, std::string&> {
	enum MessageType {
		MULTICAST = 1,
		UNICAST = 2,
		ANNOUNCE = 3,
		KEY_REQUEST = 4,
		KEY_SEND = 5,
		CLIENT_MESSAGES_DONE = 6
	};

	std::map<std::string, RR_Factory::MessageType> MessageMap;

	//This map is used as a cache to avoid repetitive handler creation in heap
	std::map<MessageType, boost::shared_ptr<sim_mob::Handler> > HandlerMap;

public:
	RR_Factory();
	virtual ~RR_Factory();
	bool createMessage(std::string &str, std::vector<sim_mob::comm::MsgPtr>&output);
	boost::shared_ptr<sim_mob::Handler>  getHandler(MessageType);
};

}}
