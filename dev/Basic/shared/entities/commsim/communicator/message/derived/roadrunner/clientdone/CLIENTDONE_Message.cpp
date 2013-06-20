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
MSG_CLIENTDONE::MSG_CLIENTDONE(msg_data_t& data_): RoadrunnerMessage(data_)
{

}
Handler * MSG_CLIENTDONE::newHandler()
{
	return 0;
}
}/* namespace roadrunner */
} /* namespace sim_mob */



