/*
 * RoadrunnerMessage.cpp
 *
 *  Created on: May 13, 2013
 *      Author: vahid
 */

#include "RoadrunnerMessage.hpp"

namespace sim_mob {
namespace roadrunner {

RoadrunnerMessage::RoadrunnerMessage(std::string data_):data(data_){
}

RoadrunnerMessage::~RoadrunnerMessage() {
	// TODO Auto-generated destructor stub
}

hdlr_ptr RoadrunnerMessage::supplyHandler() {
	return handler;
}

void RoadrunnerMessage::setHandler(hdlr_ptr handler_)
{
	handler = handler_;
}

} /* namespace roadrunner */
} /* namespace sim_mob */
