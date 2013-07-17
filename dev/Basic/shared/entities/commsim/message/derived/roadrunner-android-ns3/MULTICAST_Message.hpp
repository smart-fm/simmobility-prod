/*
 * MULTICAST_Message.h
 *
 *  Created on: May 9, 2013
 *      Author: vahid
 *      multicast messages come from android clients(only) and
 *      are supposed to be routed to ns3
 */

#ifndef MULTICAST_MESSAGE_HPP_
#define MULTICAST_MESSAGE_HPP_
#include "entities/commsim/message/base/Message.hpp"
namespace sim_mob {
namespace rr_android_ns3 {

class MSG_MULTICAST : public sim_mob::comm::Message<msg_data_t> {
	//...
public:
	Handler * newHandler();
	MSG_MULTICAST(msg_data_t data_);
};


//Handler for the above message
class HDL_MULTICAST : public Handler {

public:
	void handle(msg_ptr message_,Broker*);
};

}/* namespace rr_android_ns3 */
} /* namespace sim_mob */
#endif /* MULTICAST_MESSAGE_H_ */
