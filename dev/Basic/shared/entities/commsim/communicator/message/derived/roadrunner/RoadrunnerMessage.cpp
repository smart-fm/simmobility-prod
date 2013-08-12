/*
 * RoadrunnerMessage.cpp
 *
 *  Created on: May 13, 2013
 *      Author: vahid
 */

#include "RoadrunnerMessage.hpp"

namespace sim_mob {
namespace roadrunner {

RoadrunnerMessage::RoadrunnerMessage(msg_data_t& data_):AbstractCommMessage(data_){
}

RoadrunnerMessage::~RoadrunnerMessage() {
	// TODO Auto-generated destructor stub
}

} /* namespace roadrunner */
} /* namespace sim_mob */
