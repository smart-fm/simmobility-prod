//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

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
