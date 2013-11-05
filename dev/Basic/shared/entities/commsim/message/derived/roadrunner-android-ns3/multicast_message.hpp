//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * MULTICAST_Message.h
 *
 *  Created on: May 9, 2013
 *      Author: vahid
 *      multicast messages come from android clients(only) and
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
class ANDROID_MSG_MULTICAST : public sim_mob::comm::Message {
	//...
public:
	Handler * newHandler();
	ANDROID_MSG_MULTICAST(Json::Value data_);
};


//Handler for the above message
class ANDROID_HDL_MULTICAST : public Handler {

public:
	void handle(msg_ptr message_,Broker*);
};
/***************************************************************************************************************************************************
 *****************************   NS3   ************************************************************************************************************
 **************************************************************************************************************************************************/

class NS3_MSG_MULTICAST : public sim_mob::comm::Message {
	//...
public:
	Handler * newHandler();
	NS3_MSG_MULTICAST(Json::Value data_);
};


//Handler for the above message
class NS3_HDL_MULTICAST : public Handler {

public:
	void handle(msg_ptr message_,Broker*);
};

}/* namespace rr_android_ns3 */
} /* namespace sim_mob */
