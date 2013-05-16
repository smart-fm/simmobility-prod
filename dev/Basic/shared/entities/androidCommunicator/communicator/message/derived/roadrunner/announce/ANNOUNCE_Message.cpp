/*
 * ANNOUNCE_Message.cpp
 *
 *  Created on: May 9, 2013
 *      Author: vahid
 */

#include "ANNOUNCE_Handler.hpp"
//#include "../RoadrunnerMessage.hpp"
namespace sim_mob {
class Handler;

namespace roadrunner
{
class HDL_ANNOUNCE;
MSG_ANNOUNCE::MSG_ANNOUNCE(std::string data_): RoadrunnerMessage(data_)
{

}
Handler * MSG_ANNOUNCE::newHandler()
{
	return new HDL_ANNOUNCE();
}
}/* namespace roadrunner */
} /* namespace sim_mob */



