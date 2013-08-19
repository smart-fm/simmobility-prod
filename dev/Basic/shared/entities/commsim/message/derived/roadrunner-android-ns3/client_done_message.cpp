/*
 * CLIENTDONE_Message.cpp
 *
 *  Created on: May 9, 2013
 *      Author: vahid
 */

//#include "CLIENTDONE_Handler.hpp"
#include "client_done_message.hpp"
namespace sim_mob {
class Handler;

namespace rr_android_ns3
{
class HDL_CLIENTDONE;
MSG_CLIENTDONE::MSG_CLIENTDONE(msg_data_t& data_): Message(data_)
{

}
Handler * MSG_CLIENTDONE::newHandler()
{
	return 0;
}
}/* namespace rr_android_ns3 */
} /* namespace sim_mob */



