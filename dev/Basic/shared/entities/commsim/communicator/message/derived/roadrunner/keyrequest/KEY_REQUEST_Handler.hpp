/*
 * KEY_REQUEST_Handler.h
 *
 *  Created on: May 9, 2013
 *      Author: vahid
 */

#ifndef KEY_REQUEST_HANDLER_HPP_
#define KEY_REQUEST_HANDLER_HPP_

//#include "entities/commsim/communicator/message/base/Handler.hpp"
//#include "entities/commsim/communicator/message/base/Message.hpp"
#include "KEY_REQUEST_Message.hpp"
namespace sim_mob {
namespace roadrunner
{
//class MSG_KEY_REQUEST;
class HDL_KEY_REQUEST : public Handler {

public:
//	HDL_KEY_REQUEST();
	void handle(msg_ptr message_);
};
}/* namespace roadrunner */
} /* namespace sim_mob */
#endif /* KEY_REQUEST_HANDLER_H_ */
