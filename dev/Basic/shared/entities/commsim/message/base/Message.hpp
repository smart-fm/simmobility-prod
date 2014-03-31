//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * this file is inclusion place for custome Messgae classes
 * whose type will be used in defining templated handlers
 * This generic class is basically assumed to serve as a wrapper around a
 * data string with Json format.
 */

#pragma once

#include "entities/commsim/message/Types.hpp"

namespace sim_mob {


///MessageFactory (no documentation provided).
/*template <class RET,class MSG>
class MessageFactory {
public:
	virtual ~MessageFactory() {}
	virtual void createMessage(const MSG& input,RET& output) const = 0;
};
*/

namespace comm {

///Base Message (no documentation provided).
/*class Message {
	sim_mob::comm::MsgData data;
	sim_mob::comm::MsgHandler handler;
public:
	//Message();
	Message(const sim_mob::comm::MsgData& data_);

	sim_mob::comm::MsgHandler supplyHandler();

	void setHandler( sim_mob::comm::MsgHandler handler_);

	sim_mob::comm::MsgData& getData();
};*/

}}
