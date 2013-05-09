/*
 * ANNOUNCE_Message.h
 *
 *  Created on: May 9, 2013
 *      Author: vahid
 */

#ifndef ANNOUNCE_MESSAGE_HPP_
#define ANNOUNCE_MESSAGE_HPP_
#include "ANNOUNCE_Handler.hpp"
namespace sim_mob {

class M_ANNOUNCE : public Message {
	//...
public:
	Handler * newHandler()
	{
		return new MH_ANNOUNCE(this);
	}
};

} /* namespace sim_mob */
#endif /* ANNOUNCE_MESSAGE_H_ */
