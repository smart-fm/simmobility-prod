/*
 * HandlerFactory.hpp
 *
 *  Created on: May 7, 2013
 *      Author: vahid
 */

#ifndef HANDLER_HPP_
#define HANDLER_HPP_
#include "Message.hpp"
namespace sim_mob
{
namespace comm
{
//Forward Declaration
class Message;
}
class Handler
{
public:
	virtual void handle(msg_ptr message_) = 0;
};
typedef boost::shared_ptr<Handler> hdlr_ptr;
}//namespace
#endif /* HANDLER_HPP_ */
