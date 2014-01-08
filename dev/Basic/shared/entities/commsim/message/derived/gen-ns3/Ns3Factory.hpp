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
#include "entities/commsim/serialization/JsonParser.hpp"
#include "entities/commsim/message/base/Message.hpp"

namespace sim_mob {
namespace comm {


///NS3 RR factory (no documentation provided).
class NS3_Factory : public MessageFactory<std::vector<sim_mob::comm::MsgPtr>, std::string> {
	enum MessageType {
		MULTICAST = 1,
		UNICAST = 2,
		CLIENT_MESSAGES_DONE = 6
	};

	std::map<std::string, NS3_Factory::MessageType> MessageMap;

	//This map is used as a cache to avoid repetitive handler creation in heap
	std::map<MessageType, boost::shared_ptr<sim_mob::Handler> > HandlerMap;

public:
	NS3_Factory();
	virtual ~NS3_Factory();
	bool createMessage(const std::string &str, std::vector<sim_mob::comm::MsgPtr>&output);
	boost::shared_ptr<sim_mob::Handler>  getHandler(MessageType);
};

}}

