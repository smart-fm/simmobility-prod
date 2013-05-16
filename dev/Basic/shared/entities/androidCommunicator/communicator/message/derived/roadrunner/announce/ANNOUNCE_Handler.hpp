/*
 * ANNOUNCE_Handler.h
 *
 *  Created on: May 9, 2013
 *      Author: vahid
 */

#ifndef ANNOUNCE_HANDLER_HPP_
#define ANNOUNCE_HANDLER_HPP_

//#include "entities/androidCommunicator/communicator/message/base/Handler.hpp"
//#include "entities/androidCommunicator/communicator/message/base/Message.hpp"
#include "ANNOUNCE_Message.hpp"
namespace sim_mob {
namespace roadrunner
{
//class MSG_ANNOUNCE;
class HDL_ANNOUNCE : public Handler {

public:
//	HDL_ANNOUNCE();
	void handle(msg_ptr message_);
};
}/* namespace roadrunner */
} /* namespace sim_mob */
#endif /* ANNOUNCE_HANDLER_H_ */
