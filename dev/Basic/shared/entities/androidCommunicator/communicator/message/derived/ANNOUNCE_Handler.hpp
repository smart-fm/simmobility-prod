/*
 * ANNOUNCE_Handler.h
 *
 *  Created on: May 9, 2013
 *      Author: vahid
 */

#ifndef ANNOUNCE_HANDLER_HPP_
#define ANNOUNCE_HANDLER_HPP_

#include "../base/Handler.hpp"
#include "ANNOUNCE_Message.hpp"
namespace sim_mob {
namespace android
{
class M_ANNOUNCE;
class MH_ANNOUNCE : public Handler {

public:
	//Corey! Here I had to use Base Message class. is it ok?
	/*sim_mob::comm::Message*/
	MH_ANNOUNCE(M_ANNOUNCE *message_);
	void handle();
};
}/* namespace android */
} /* namespace sim_mob */
#endif /* ANNOUNCE_HANDLER_H_ */
