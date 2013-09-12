//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * UNICAST_Message.cpp
 *
 *  Created on: May 9, 2013
 *      Author: vahid
 */

#include "UNICAST_Handler.hpp"
namespace sim_mob {
class Handler;

namespace roadrunner
{
class HDL_UNICAST;
MSG_UNICAST::MSG_UNICAST(msg_data_t& data_): RoadrunnerMessage(data_)
{

}
Handler * MSG_UNICAST::newHandler()
{
	return new HDL_UNICAST();
}
}/* namespace roadrunner */
} /* namespace sim_mob */



