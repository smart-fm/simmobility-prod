/*
 * RoadrunnerMessage.hpp
 *
 *  Created on: May 13, 2013
 *      Author: vahid
 */

#ifndef ROADRUNNERMESSAGE_HPP_
#define ROADRUNNERMESSAGE_HPP_

#include "entities/commsim/communicator/message/base/Message.hpp"
#include "entities/commsim/communicator/message/base/Handler.hpp"
namespace sim_mob {
//Forward Declaration
namespace roadrunner {

class RoadrunnerMessage : public sim_mob::comm::Message{
	//std::string type of data containing in json format
	std::string data;
	hdlr_ptr handler;
public:
	RoadrunnerMessage(std::string data_);
	hdlr_ptr supplyHandler();
//	msg_ptr getData();
	void setHandler(hdlr_ptr);
	virtual ~RoadrunnerMessage();
};

}/* namespace roadrunner */
} /* namespace sim_mob */
#endif /* ROADRUNNERMESSAGE_HPP_ */
