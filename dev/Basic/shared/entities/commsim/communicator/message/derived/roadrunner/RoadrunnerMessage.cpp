//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

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
