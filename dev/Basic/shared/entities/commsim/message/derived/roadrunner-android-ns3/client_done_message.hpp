/*
 * CLIENTDONE_Message.h
 *
 *  Created on: May 9, 2013
 *      Author: vahid
 */

#ifndef CLIENTDONE_MESSAGE_HPP_
#define CLIENTDONE_MESSAGE_HPP_
#include "entities/commsim/message/base/Message.hpp"
//#include "CLIENTDONE_Handler.hpp"
//#include "entities/commsim/message/derived/roadrunner-androidRoadrunnerMessage.hpp"

namespace sim_mob {
namespace rr_android_ns3 {

class MSG_CLIENTDONE : public sim_mob::comm::Message<msg_data_t> {
	//...
public:
	Handler * newHandler();
	MSG_CLIENTDONE(msg_data_t& data_);
};

}/* namespace rr_android_ns3 */
} /* namespace sim_mob */
#endif /* CLIENTDONE_MESSAGE_H_ */
