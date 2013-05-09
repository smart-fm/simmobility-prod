/*
 * ANNOUNCE_Handler.h
 *
 *  Created on: May 9, 2013
 *      Author: vahid
 */

#ifndef ANNOUNCE_HANDLER_HPP_
#define ANNOUNCE_HANDLER_HPP_

#include "base/Handler.hpp"
namespace sim_mob {

class MH_ANNOUNCE : public Handler {

public:
	//Corey! Here I had to use Base Message class. is it ok?
	MH_ANNOUNCE(/*M_ANNOUNCE*/ Message *message_): Handler(message_)
	{
	}
	void handle(){}
};

} /* namespace sim_mob */
#endif /* ANNOUNCE_HANDLER_H_ */
