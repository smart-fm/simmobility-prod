/*
 * RRMSGFactory.hpp
 *
 *  Created on: May 13, 2013
 *      Author: vahid
 */

#ifndef RRMSGFACTORY_HPP_
#define RRMSGFACTORY_HPP_

#include "entities/androidCommunicator/communicator/message/base/MessageFactory.hpp"
#include "announce/ANNOUNCE_Handler.hpp"
#include <map>
namespace sim_mob {
namespace roadrunner{

class RR_Factory : public MessageFactory<msg_ptr, std::string>/*, public HandlerFactory*/{
	enum MessageType
	{
		ANNOUNCE = 1,
		KEY_REQUEST = 2,
		KEY_SEND = 3
	};
	std::map<std::string, MessageType> MessageMap;
	//This map is used as a cache to avoid repetitive handler creation in heap
	std::map<MessageType, hdlr_ptr > HandlerMap;
public:
	RR_Factory();

	msg_ptr createMessage(std::string str);
	hdlr_ptr  getHandler(MessageType);
};

} /* namespace roadrunner */
} /* namespace sim_mob */
#endif /* RRMSGFACTORY_HPP_ */
