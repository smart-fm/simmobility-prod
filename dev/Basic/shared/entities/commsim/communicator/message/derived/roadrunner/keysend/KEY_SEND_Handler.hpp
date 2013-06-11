/*
 * KEY_SEND_Handler.h
 *
 *  Created on: May 9, 2013
 *      Author: vahid
 */

#ifndef KEY_SEND_HANDLER_HPP_
#define KEY_SEND_HANDLER_HPP_

//#include "entities/commsim/communicator/message/base/Handler.hpp"
//#include "entities/commsim/communicator/message/base/Message.hpp"
#include "KEY_SEND_Message.hpp"
namespace sim_mob {
namespace roadrunner
{
//class MSG_KEY_SEND;
class HDL_KEY_SEND : public Handler {

public:
//	HDL_KEY_SEND();
	void handle(msg_ptr message_,Broker*);
};
}/* namespace roadrunner */
} /* namespace sim_mob */
#endif /* KEY_SEND_HANDLER_H_ */
