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
namespace roadrunner {

class MSG_CLIENTDONE : public sim_mob::comm::Message/*sim_mob::roadrunner::RoadrunnerMessage*/ {
	//...
public:
	Handler * newHandler();
	MSG_CLIENTDONE(Json::Value& data_);
};

}/* namespace roadrunner */
} /* namespace sim_mob */
