/*
 * ANNOUNCE_Message.cpp
 *
 *  Created on: May 9, 2013
 *      Author: vahid
 */

#include "ANNOUNCE_Handler.hpp"
namespace sim_mob {
class Handler;

namespace roadrunner
{
class HDL_ANNOUNCE;
MSG_ANNOUNCE::MSG_ANNOUNCE(msg_data_t data_): RoadrunnerMessage(data_)
{

}
Handler * MSG_ANNOUNCE::newHandler()
{
	return new HDL_ANNOUNCE();
}
}/* namespace roadrunner */
} /* namespace sim_mob */



