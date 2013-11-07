//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)


#pragma once

#include <map>

#include "entities/commsim/message/Types.hpp"
#include "entities/commsim/serialization/Serialization.hpp"

#include "entities/commsim/message/base/UnicastMessage.hpp"
#include "entities/commsim/message/base/MulticastMessage.hpp"
#include "entities/commsim/message/base/ClientDoneMessage.hpp"

namespace sim_mob {
namespace roadrunner{


/***
 * This class is concerned with creating messages and message handlers based on several type strings or ids.
 * The classes RR_Factory and RR_Android_Factory are simply templatized sub-classes of this RR_FactoryBase class.
 * This was refactored to avoid duplicate code (the previous two classes were almost entirely duplicated).
 * This is NOT necessarily a clean solution; in fact, I will examine the messages and handlers to see if we
 * can share more functionality there and remove the templates entirely. ~Seth
 */
class RR_FactoryBase : public MessageFactory<std::vector<sim_mob::comm::MsgPtr>&, std::string&> {
	enum MessageType {
		MULTICAST = 1,
		UNICAST = 2,
		//ANNOUNCE = 3,
		//KEY_REQUEST = 4,
		//KEY_SEND = 5,
		CLIENT_MESSAGES_DONE = 6
	};

	std::map<std::string, RR_FactoryBase::MessageType> MessageMap;

	//This map is used as a cache to avoid repetitive handler creation in heap
	std::map<MessageType, boost::shared_ptr<sim_mob::Handler> > HandlerMap;

public:
	RR_FactoryBase(bool useNs3);
	virtual ~RR_FactoryBase();

	//creates a message with correct format + assigns correct handler
	//todo improve the function to handle array of messages stored in the input string
	bool createMessage(std::string &str, std::vector<sim_mob::comm::MsgPtr>&output);

	//gets a handler either from a cache or by creating a new one
	boost::shared_ptr<sim_mob::Handler>  getHandler(MessageType);

private:
	bool useNs3;
};


}}
