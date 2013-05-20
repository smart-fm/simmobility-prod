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
MSG_KEY_SEND::MSG_KEY_SEND(std::string data_): RoadrunnerMessage(data_)
{

}
Handler * MSG_KEY_SEND::newHandler()
{
	return new HDL_KEY_SEND();
}
}/* namespace roadrunner */
} /* namespace sim_mob */



