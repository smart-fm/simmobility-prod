//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * KEY_SEND_Message.cpp
 *
 *  Created on: May 9, 2013
 *      Author: vahid
 */

#include "KEY_SEND_Handler.hpp"
namespace sim_mob {
class Handler;

namespace roadrunner
{
class HDL_KEY_SEND;
MSG_KEY_SEND::MSG_KEY_SEND(msg_data_t& data_): RoadrunnerMessage(data_)
{

}
Handler * MSG_KEY_SEND::newHandler()
{
	return new HDL_KEY_SEND();
}
}/* namespace roadrunner */
} /* namespace sim_mob */



