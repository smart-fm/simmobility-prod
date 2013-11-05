//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * CLIENTDONE_Message.h
 *
 *  Created on: May 9, 2013
 *      Author: vahid
 */

#pragma once

#include "entities/commsim/message/base/Message.hpp"

//#include "CLIENTDONE_Handler.hpp"
//#include "entities/commsim/message/derived/roadrunner-androidRoadrunnerMessage.hpp"

namespace sim_mob {
namespace rr_android_ns3 {

class MSG_CLIENTDONE : public sim_mob::comm::Message {
	//...
public:
	Handler * newHandler();
	MSG_CLIENTDONE(Json::Value& data_);
};

}/* namespace rr_android_ns3 */
} /* namespace sim_mob */
