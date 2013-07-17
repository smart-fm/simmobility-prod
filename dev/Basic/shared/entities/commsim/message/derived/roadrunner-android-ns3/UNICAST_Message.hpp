/*
 * UNICAST_Message.h
 *
 *  Created on: May 9, 2013
 *      Author: vahid
 *      unicast messages come from android clients(only) and
 *      are supposed to be routed to ns3
 */

#ifndef UNICAST_MESSAGE_HPP_
#define UNICAST_MESSAGE_HPP_
#include "entities/commsim/message/base/Message.hpp"

namespace sim_mob {
namespace rr_android_ns3 {

class MSG_UNICAST : public sim_mob::comm::Message<msg_data_t> {
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
