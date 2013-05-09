/*
 * Handler.cpp
 *
 *  Created on: May 9, 2013
 *      Author: vahid
 */
#include "Handler.hpp"
#include "Message.hpp"
namespace sim_mob
{

Handler::Handler(sim_mob::comm::Message* message_):message(message_){
}

sim_mob::comm::Message * Handler::getMessage()
{
	return message;
}

}



