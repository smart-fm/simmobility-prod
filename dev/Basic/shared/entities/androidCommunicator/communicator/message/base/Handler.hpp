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
namespace comm
{
//Forward Declaration
class Message;
}
class Handler
{
	sim_mob::comm::Message *message;
public:
	Handler(sim_mob::comm::Message* message_);
	virtual void handle() = 0;
	sim_mob::comm::Message * getMessage();
};
}//namespace
#endif /* HANDLER_HPP_ */
