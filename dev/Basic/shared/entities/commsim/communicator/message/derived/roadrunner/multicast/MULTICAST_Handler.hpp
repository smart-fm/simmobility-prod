/*
 * MULTICAST_Handler.h
 *
 *  Created on: May 9, 2013
 *      Author: vahid
 */

#ifndef MULTICAST_HANDLER_HPP_
#define MULTICAST_HANDLER_HPP_

//#include "entities/commsim/communicator/message/base/Handler.hpp"
//#include "entities/commsim/communicator/message/base/Message.hpp"
#include "MULTICAST_Message.hpp"
namespace sim_mob {
namespace roadrunner
{
//class MSG_MULTICAST;
class HDL_MULTICAST : public Handler {

public:
//	HDL_MULTICAST();
	void handle(msg_ptr message_,Broker*);
};
}/* namespace roadrunner */
} /* namespace sim_mob */
#endif /* MULTICAST_HANDLER_H_ */
