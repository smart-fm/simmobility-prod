/*
 * MULTICAST_Message.h
 *
 *  Created on: May 9, 2013
 *      Author: vahid
 */

#ifndef MULTICAST_MESSAGE_HPP_
#define MULTICAST_MESSAGE_HPP_
//#include "entities/commsim/communicator/message/base/Message.hpp"
//#include "MULTICAST_Handler.hpp"
#include "entities/commsim/communicator/message/derived/roadrunner/RoadrunnerMessage.hpp"

namespace sim_mob {
namespace roadrunner {

class MSG_MULTICAST : public sim_mob::roadrunner::RoadrunnerMessage {
	//...
public:
	Handler * newHandler();
	MSG_MULTICAST(msg_data_t data_);
};

}/* namespace roadrunner */
} /* namespace sim_mob */
#endif /* MULTICAST_MESSAGE_H_ */
