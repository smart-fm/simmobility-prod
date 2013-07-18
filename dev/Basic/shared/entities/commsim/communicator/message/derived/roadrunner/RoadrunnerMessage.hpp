/*
 * RoadrunnerMessage.hpp
 *
 *  Created on: May 13, 2013
 *      Author: vahid
 */

#pragma once

#include "entities/commsim/communicator/message/base/Message.hpp"
#include "entities/commsim/communicator/message/base/Handler.hpp"
namespace sim_mob {
//Forward Declaration
namespace roadrunner {

class RoadrunnerMessage : public sim_mob::comm::Message<msg_data_t> {
	//std::string type of data containing in json format

public:
	RoadrunnerMessage(msg_data_t& data_);
	hdlr_ptr supplyHandler();
	virtual ~RoadrunnerMessage();
};

}/* namespace roadrunner */
} /* namespace sim_mob */
