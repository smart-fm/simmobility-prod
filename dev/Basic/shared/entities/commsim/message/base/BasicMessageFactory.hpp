//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)


#pragma once

#include <map>

#include "entities/commsim/message/Types.hpp"
#include "entities/commsim/message/base/Message.hpp"
#include "entities/commsim/serialization/JsonParser.hpp"


namespace sim_mob {

/***
 * This class can create messages that all clients must know about; in particular, the WHOAMI message.
 * Note that the WHOAMI message has no handler, as it is filtered through the Broker.
 */
class BasicMessageFactory : public MessageFactory<std::vector<sim_mob::comm::MsgPtr>, std::string> {
	enum MessageType {
		WHOAMI = 1
	};

public:
	virtual ~BasicMessageFactory();

	void createMessage(const std::string& input, std::vector<sim_mob::comm::MsgPtr>& output) const;
};


}
