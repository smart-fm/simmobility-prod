/*
 * HandlerFactory.hpp
 *
 *  Created on: May 7, 2013
 *      Author: vahid
 */

#ifndef HANDLERFACTORY_HPP_
#define HANDLERFACTORY_HPP_

#include "Message.hpp"

namespace sim_mob {
class Message;
class Handler;
//my Base Handler Factory

class HandlerFactory {
public:
	virtual Handler * create(Message * message){
		return message->newHandler();
	}
};

} /* namespace sim_mob */
#endif /* HANDLERFACTORY_HPP_ */
