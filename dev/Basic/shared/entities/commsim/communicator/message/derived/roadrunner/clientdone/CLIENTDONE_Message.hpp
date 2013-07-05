/*
 * CLIENTDONE_Message.h
 *
 *  Created on: May 9, 2013
 *      Author: vahid
 */

#ifndef CLIENTDONE_MESSAGE_HPP_
#define CLIENTDONE_MESSAGE_HPP_
//#include "entities/commsim/communicator/message/base/Message.hpp"
//#include "CLIENTDONE_Handler.hpp"
#include "entities/commsim/communicator/message/derived/roadrunner/RoadrunnerMessage.hpp"

namespace sim_mob {
namespace roadrunner {

class MSG_CLIENTDONE : public sim_mob::roadrunner::RoadrunnerMessage {
	//...
public:
	Handler * newHandler();
	MSG_CLIENTDONE(msg_data_t& data_);
};

}/* namespace roadrunner */
} /* namespace sim_mob */
#endif /* CLIENTDONE_MESSAGE_H_ */
