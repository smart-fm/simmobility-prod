/*
 * UNICAST_Message.h
 *
 *  Created on: May 9, 2013
 *      Author: vahid
 *      unicast messages come from android clients(only) and
 *      are supposed to be routed to ns3
 */

#ifndef ANDROID_UNICAST_MESSAGE_HPP_
#define ANDROID_UNICAST_MESSAGE_HPP_
#include "entities/commsim/message/base/Message.hpp"

namespace sim_mob {
namespace rr_android_ns3 {

class ANDROID_MSG_UNICAST : public sim_mob::comm::Message<msg_data_t> {
	//...
public:
	Handler * newHandler();
	ANDROID_MSG_UNICAST(msg_data_t& data_);
};

//Handler to the above message
class ANDROID_HDL_UNICAST : public Handler {

public:
//	ANDROID_HDL_UNICAST();
	void handle(msg_ptr message_,Broker*);
};

}/* namespace roadrunner */
} /* namespace sim_mob */
#endif /* ANDROID_UNICAST_MESSAGE_H_ */
