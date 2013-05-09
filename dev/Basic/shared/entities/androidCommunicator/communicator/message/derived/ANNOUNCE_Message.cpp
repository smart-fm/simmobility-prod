/*
 * ANNOUNCE_Message.cpp
 *
 *  Created on: May 9, 2013
 *      Author: vahid
 */

#include "ANNOUNCE_Message.hpp"
namespace sim_mob {
class Handler;
namespace android
{
Handler * M_ANNOUNCE::newHandler()
{
	return new MH_ANNOUNCE(this);
}
}/* namespace android */
} /* namespace sim_mob */



