//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * MULTICAST_Message.h
 *
 *  Created on: May 9, 2013
 *      Author: vahid
 */

#pragma once

//#include "entities/commsim/message/derived/roadrunner-androidRoadrunnerMessage.hpp"

#include "entities/commsim/message/base/Message.hpp"
#include "entities/commsim/message/base/Handler.hpp"

namespace sim_mob {
namespace roadrunner {

//class MSG_MULTICAST : public sim_mob::roadrunner::RoadrunnerMessage {
class MSG_MULTICAST : public sim_mob::comm::Message {
	//...
public:
	Handler * newHandler();
	MSG_MULTICAST(Json::Value data_);
};


//Handler for the above message
class HDL_MULTICAST : public Handler {

public:
	void handle(msg_ptr message_,Broker*);
};

}/* namespace roadrunner */
} /* namespace sim_mob */
