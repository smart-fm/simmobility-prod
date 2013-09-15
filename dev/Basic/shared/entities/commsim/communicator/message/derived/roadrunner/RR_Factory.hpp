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

#include "entities/commsim/communicator/message/base/MessageFactory.hpp"
#include "entities/commsim/communicator/serialization/Serialization.hpp"
#include "entities/commsim/communicator/message/base/Message.hpp"
//#include "announce/ANNOUNCE_Handler.hpp"
//#include "keyrequest/KEY_REQUEST_Handler.hpp"
//#include "keysend/KEY_SEND_Handler.hpp"
#include "multicast/MULTICAST_Handler.hpp"
#include "unicast/UNICAST_Handler.hpp"
#include "clientdone/CLIENTDONE_Message.hpp"
#include <map>

namespace sim_mob {
namespace roadrunner{

class RR_Factory : public MessageFactory<std::vector<msg_ptr>&, std::string&>/*MessageFactory<output, input>y*/{
	enum MessageType
	{
		MULTICAST = 1,
		UNICAST = 2,
		ANNOUNCE = 3,
		KEY_REQUEST = 4,
		KEY_SEND = 5,
		CLIENT_MESSAGES_DONE = 6
	};
	std::map<std::string, RR_Factory::MessageType> MessageMap;
	//This map is used as a cache to avoid repetitive handler creation in heap
	std::map<MessageType, hdlr_ptr > HandlerMap;
public:
	RR_Factory();
	virtual ~RR_Factory();
	bool createMessage(std::string &str, std::vector<msg_ptr>&output);
	hdlr_ptr  getHandler(MessageType);
};

} /* namespace roadrunner */
} /* namespace sim_mob */
