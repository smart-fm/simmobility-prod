/*
 * KEY_REQUEST_Message.cpp
 *
 *  Created on: May 9, 2013
 *      Author: vahid
 */

#include "KEY_REQUEST_Handler.hpp"
namespace sim_mob {
class Handler;

namespace roadrunner
{
class HDL_KEY_REQUEST;
MSG_KEY_REQUEST::MSG_KEY_REQUEST(msg_data_t& data_): RoadrunnerMessage(data_)
{

}
Handler * MSG_KEY_REQUEST::newHandler()
{
	return new HDL_KEY_REQUEST();
}
}/* namespace roadrunner */
} /* namespace sim_mob */



