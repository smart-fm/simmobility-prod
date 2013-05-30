/*
 * RRMSGFactory.hpp
 *
 *  Created on: May 13, 2013
 *      Author: vahid
 */

#ifndef  RR_FACTORY_HPP_
#define RR_FACTORY_HPP_

#include "entities/commsim/communicator/message/base/MessageFactory.hpp"
#include "entities/commsim/communicator/serialization/Serialization.hpp"
#include "entities/commsim/communicator/message/base/Message.hpp"
#include "announce/ANNOUNCE_Handler.hpp"
#include "keyrequest/KEY_REQUEST_Handler.hpp"
#include "keysend/KEY_SEND_Handler.hpp"
#include <map>

namespace sim_mob {
namespace roadrunner{

class RR_Factory : public MessageFactory<std::vector<msg_ptr>, std::string>/*, public HandlerFactory*/{
	enum MessageType
	{
		ANNOUNCE = 1,
		KEY_REQUEST = 2,
		KEY_SEND = 3
	};
	std::map<std::string, RR_Factory::MessageType> MessageMap;
	//This map is used as a cache to avoid repetitive handler creation in heap
	std::map<MessageType, hdlr_ptr > HandlerMap;
public:
	RR_Factory();
	virtual ~RR_Factory();
	std::vector<msg_ptr> createMessage(std::string str);
	hdlr_ptr  getHandler(MessageType);
};

} /* namespace roadrunner */
} /* namespace sim_mob */
#endif /* RRMSGFACTORY_HPP_ */
