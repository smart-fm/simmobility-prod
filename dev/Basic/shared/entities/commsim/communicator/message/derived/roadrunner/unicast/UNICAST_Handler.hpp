/*
 * UNICAST_Handler.h
 *
 *  Created on: May 9, 2013
 *      Author: vahid
 */

#pragma once

//#include "entities/commsim/communicator/message/base/Handler.hpp"
//#include "entities/commsim/communicator/message/base/Message.hpp"
#include "UNICAST_Message.hpp"
namespace sim_mob {
namespace roadrunner
{
//class MSG_UNICAST;
class HDL_UNICAST : public Handler {

public:
//	HDL_UNICAST();
	void handle(msg_ptr message_,Broker*);
};
}/* namespace roadrunner */
} /* namespace sim_mob */
