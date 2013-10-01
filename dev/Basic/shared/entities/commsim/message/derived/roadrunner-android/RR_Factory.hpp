//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * RRMSGFactory.hpp
 *
 *  Created on: May 13, 2013
 *      Author: vahid
 */

#ifndef  RR_ANDROID_FACTORY_HPP_
#define  RR_ANDROID_FACTORY_HPP_

#include "entities/commsim/serialization/Serialization.hpp"
#include "MULTICAST_Message.hpp"
#include "UNICAST_Message.hpp"
#include "CLIENTDONE_Message.hpp"
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
#endif /* RRMSGFACTORY_HPP_ */
