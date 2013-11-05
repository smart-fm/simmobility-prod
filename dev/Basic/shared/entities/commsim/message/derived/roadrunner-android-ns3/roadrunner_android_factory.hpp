/*
 * RRMSGFactory.hpp
 *
 *  Created on: May 13, 2013
 *      Author: vahid
 */

#ifndef  RR_ANDROID_NS3_FACTORY_HPP_
#define  RR_ANDROID_NS3_FACTORY_HPP_

#include "entities/commsim/serialization/Serialization.hpp"
#include "multicast_message.hpp"
#include "unicast_message.hpp"
#include "client_done_message.hpp"
#include <map>

namespace sim_mob {
namespace rr_android_ns3{
/***************************************************************************************************************************************************
 *****************************   ANDROID   ************************************************************************************************************
 **************************************************************************************************************************************************/
class RR_Android_Factory : public MessageFactory<std::vector<msg_ptr>&, std::string&>/*MessageFactory<output, input>y*/{
	enum MessageType
	{
		MULTICAST = 1,
		UNICAST = 2,
		CLIENT_MESSAGES_DONE = 6
	};
	std::map<std::string, RR_Android_Factory::MessageType> MessageMap;
	//This map is used as a cache to avoid repetitive handler creation in heap
	std::map<MessageType, boost::shared_ptr<sim_mob::Handler> > HandlerMap;
public:
	RR_Android_Factory();
	virtual ~RR_Android_Factory();
	bool createMessage(std::string &str, std::vector<msg_ptr>&output);
	boost::shared_ptr<sim_mob::Handler>  getHandler(MessageType);
};
/***************************************************************************************************************************************************
 *****************************   NS3   ************************************************************************************************************
 **************************************************************************************************************************************************/
class RR_NS3_Factory : public MessageFactory<std::vector<msg_ptr>&, std::string&>/*MessageFactory<output, input>y*/{
	enum MessageType
	{
		MULTICAST = 1,
		UNICAST = 2,
		CLIENT_MESSAGES_DONE = 6
	};
	std::map<std::string, RR_NS3_Factory::MessageType> MessageMap;
	//This map is used as a cache to avoid repetitive handler creation in heap
	std::map<MessageType, boost::shared_ptr<sim_mob::Handler> > HandlerMap;
public:
	RR_NS3_Factory();
	virtual ~RR_NS3_Factory();
	bool createMessage(std::string &str, std::vector<msg_ptr>&output);
	boost::shared_ptr<sim_mob::Handler>  getHandler(MessageType);
};

} /* namespace rr_android_ns3 */
} /* namespace sim_mob */
#endif /* RR_ANDROID_NS3_FACTORY_HPP_ */
