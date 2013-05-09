/*
 * HandlerFactory.cpp
 *
 *  Created on: May 9, 2013
 *      Author: vahid
 */
#include "HandlerFactory.hpp"
#include "Message.hpp"
namespace sim_mob
{

Handler * HandlerFactory::create(sim_mob::comm::Message * message){
	return message->newHandler();
}
}//namespace



