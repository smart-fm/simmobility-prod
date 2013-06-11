/*
 * UNICAST_Message.h
 *
 *  Created on: May 9, 2013
 *      Author: vahid
 */

#ifndef UNICAST_MESSAGE_HPP_
#define UNICAST_MESSAGE_HPP_
//#include "entities/commsim/communicator/message/base/Message.hpp"
//#include "UNICAST_Handler.hpp"
#include "entities/commsim/communicator/message/derived/roadrunner/RoadrunnerMessage.hpp"

namespace sim_mob {
namespace roadrunner {

class MSG_UNICAST : public sim_mob::roadrunner::RoadrunnerMessage {
	//...
public:
	Handler * newHandler();
	MSG_UNICAST(msg_data_t& data_);
};

}/* namespace roadrunner */
} /* namespace sim_mob */
#endif /* UNICAST_MESSAGE_H_ */
