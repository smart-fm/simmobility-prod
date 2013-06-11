/*
 * KEY_SEND_Message.h
 *
 *  Created on: May 9, 2013
 *      Author: vahid
 */

#ifndef KEY_SEND_MESSAGE_HPP_
#define KEY_SEND_MESSAGE_HPP_
//#include "entities/commsim/communicator/message/base/Message.hpp"
//#include "KEY_SEND_Handler.hpp"
#include "entities/commsim/communicator/message/derived/roadrunner/RoadrunnerMessage.hpp"

namespace sim_mob {
namespace roadrunner {

class MSG_KEY_SEND : public sim_mob::roadrunner::RoadrunnerMessage {
	//...
public:
	Handler * newHandler();
	MSG_KEY_SEND(msg_data_t& data_);
};

}/* namespace roadrunner */
} /* namespace sim_mob */
#endif /* KEY_SEND_MESSAGE_H_ */
