//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * CLIENTDONE_Message.cpp
 *
 *  Created on: May 9, 2013
 *      Author: vahid
 */

//#include "CLIENTDONE_Handler.hpp"
#include "CLIENTDONE_Message.hpp"
namespace sim_mob {
class Handler;

namespace roadrunner
{
class HDL_CLIENTDONE;

MSG_CLIENTDONE::MSG_CLIENTDONE(Json::Value& data_): /*Roadrunner*/Message(data_)
{

}
Handler * MSG_CLIENTDONE::newHandler()
{
	return 0;
}
}/* namespace roadrunner */
} /* namespace sim_mob */



