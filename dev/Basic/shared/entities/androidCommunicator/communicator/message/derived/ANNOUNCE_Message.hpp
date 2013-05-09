/*
 * ANNOUNCE_Message.h
 *
 *  Created on: May 9, 2013
 *      Author: vahid
 */

#ifndef ANNOUNCE_MESSAGE_HPP_
#define ANNOUNCE_MESSAGE_HPP_
#include "ANNOUNCE_Handler.hpp"
#include "../base/Handler.hpp"
#include "../base/Message.hpp"
namespace sim_mob {
namespace android
{

class M_ANNOUNCE : public sim_mob::comm::Message {
	//...
public:
	Handler * newHandler();
};
}/* namespace android */
} /* namespace sim_mob */
#endif /* ANNOUNCE_MESSAGE_H_ */
