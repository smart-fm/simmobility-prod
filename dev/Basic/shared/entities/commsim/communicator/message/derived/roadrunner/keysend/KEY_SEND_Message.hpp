//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * KEY_SEND_Message.h
 *
 *  Created on: May 9, 2013
 *      Author: vahid
 */

#pragma once

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
