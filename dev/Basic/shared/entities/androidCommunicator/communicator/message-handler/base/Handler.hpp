/*
 * HandlerFactory.hpp
 *
 *  Created on: May 7, 2013
 *      Author: vahid
 */

#ifndef HANDLER_HPP_
#define HANDLER_HPP_
namespace sim_mob
{
//Forward Declaration
class Message;

class Handler
{
	Message *message;
public:
	Handler(Message* message_):message(message_){
	}

	virtual void handle() = 0;
	Message * getMessage()
	{
		return message;
	}
};
}//namespace
#endif /* HANDLER_HPP_ */
