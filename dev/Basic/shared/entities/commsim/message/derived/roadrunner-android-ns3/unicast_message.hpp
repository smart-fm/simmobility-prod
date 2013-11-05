//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * UNICAST_Message.h
 *
 *  Created on: May 9, 2013
 *      Author: vahid
 *      unicast messages come from android clients(only) and
 *      are supposed to be routed to ns3
 */

#pragma once

#include "entities/commsim/message/base/Message.hpp"
#include "entities/commsim/message/base/Handler.hpp"

namespace sim_mob {
namespace rr_android_ns3 {

/***************************************************************************************************************************************************
 *****************************   ANDROID   ************************************************************************************************************
 **************************************************************************************************************************************************/
class ANDROID_MSG_UNICAST : public sim_mob::comm::Message {
	//...
public:
	Handler * newHandler();
	ANDROID_MSG_UNICAST(Json::Value& data_);
};

//Handler to the above message
class ANDROID_HDL_UNICAST : public Handler {

public:
//	ANDROID_HDL_UNICAST();
	void handle(msg_ptr message_,Broker*);
};

/***************************************************************************************************************************************************
 *****************************   NS3   ************************************************************************************************************
 **************************************************************************************************************************************************/

class NS3_MSG_UNICAST : public sim_mob::comm::Message {
	//...
public:
	Handler * newHandler();
	NS3_MSG_UNICAST(Json::Value& data_);
};

//Handler to the above message
class NS3_HDL_UNICAST : public Handler {

public:
//	NS3_HDL_UNICAST();
	void handle(msg_ptr message_,Broker*);
};
}/* namespace roadrunner */
} /* namespace sim_mob */
