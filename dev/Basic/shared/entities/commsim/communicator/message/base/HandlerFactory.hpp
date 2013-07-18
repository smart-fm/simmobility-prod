/*
 * HandlerFactory.hpp
 *
 *  Created on: May 7, 2013
 *      Author: vahid
 */

#pragma once

#include "Handler.hpp"

namespace sim_mob {
namespace comm
{
template<class T>
class Message;
}
//class Handler;
//my Base Handler Factory

class HandlerFactory {
public:
	virtual hdlr_ptr  supplyHandler(msg_ptr message) = 0;
};

} /* namespace sim_mob */
