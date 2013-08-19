/*
 * HandlerFactory.hpp
 *
 *  Created on: May 7, 2013
 *      Author: vahid
 */

#pragma once

#include "Message.hpp"
namespace sim_mob
{
class Broker;
//namespace comm
//{
////Forward Declaration
//class Message;
//}
class Handler
{
public:
	virtual void handle(msg_ptr message_,Broker*) = 0;
};
}//namespace
