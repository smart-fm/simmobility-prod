/*
 * KEY_REQUEST_Message.h
 *
 *  Created on: May 9, 2013
 *      Author: vahid
 */

#ifndef KEY_REQUEST_MESSAGE_HPP_
#define KEY_REQUEST_MESSAGE_HPP_
//#include "entities/commsim/communicator/message/base/Message.hpp"
//#include "KEY_REQUEST_Handler.hpp"
#include "entities/commsim/communicator/message/derived/roadrunner/RoadrunnerMessage.hpp"

namespace sim_mob {
namespace roadrunner {

class MSG_KEY_REQUEST : public sim_mob::roadrunner::RoadrunnerMessage {
	//...
public:
	Handler * newHandler();
	MSG_KEY_REQUEST(msg_data_t& data_);
};

}/* namespace roadrunner */
} /* namespace sim_mob */
#endif /* KEY_REQUEST_MESSAGE_H_ */
