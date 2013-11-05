//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * UNICAST_Message.h
 *
 *  Created on: May 9, 2013
 *      Author: vahid
 */

#pragma once

#include "entities/commsim/message/base/Message.hpp"
#include "entities/commsim/message/base/Handler.hpp"

//#include "UNICAST_Handler.hpp"
//#include "entities/commsim/message/derived/roadrunner-androidRoadrunnerMessage.hpp"

namespace sim_mob {
namespace roadrunner {

class MSG_UNICAST : public sim_mob::comm::Message/*sim_mob::roadrunner::RoadrunnerMessage*/ {
	//...
public:
	Handler * newHandler();
	MSG_UNICAST(Json::Value& data_);
};

//Handler to the above message
class HDL_UNICAST : public Handler {

public:
//	HDL_UNICAST();
	void handle(msg_ptr message_,Broker*);
};

}/* namespace roadrunner */
} /* namespace sim_mob */

