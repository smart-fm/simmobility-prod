/*
 * UNICAST_Message.h
 *
 *  Created on: May 9, 2013
 *      Author: vahid
 */

#ifndef UNICAST_MESSAGE_HPP_
#define UNICAST_MESSAGE_HPP_
#include "entities/commsim/message/base/Message.hpp"
//#include "UNICAST_Handler.hpp"
//#include "entities/commsim/message/derived/roadrunner-androidRoadrunnerMessage.hpp"

namespace sim_mob {
namespace roadrunner {

class MSG_UNICAST : public sim_mob::comm::Message<msg_data_t>/*sim_mob::roadrunner::RoadrunnerMessage*/ {
	//...
public:
	Handler * newHandler();
	MSG_UNICAST(msg_data_t& data_);
};

//Handler to the above message
class HDL_UNICAST : public Handler {

public:
//	HDL_UNICAST();
	void handle(msg_ptr message_,Broker*);
};

}/* namespace roadrunner */
} /* namespace sim_mob */
#endif /* UNICAST_MESSAGE_H_ */
