/*
 * ANNOUNCE_Message.h
 *
 *  Created on: May 9, 2013
 *      Author: vahid
 */

#ifndef ANNOUNCE_MESSAGE_HPP_
#define ANNOUNCE_MESSAGE_HPP_
//#include "entities/androidCommunicator/communicator/message/base/Message.hpp"
//#include "ANNOUNCE_Handler.hpp"
#include "../RoadrunnerMessage.hpp"

namespace sim_mob {
namespace roadrunner {

class MSG_ANNOUNCE : public sim_mob::roadrunner::RoadrunnerMessage {
	//...
public:
	Handler * newHandler();
	MSG_ANNOUNCE(std::string data_);
};

}/* namespace roadrunner */
} /* namespace sim_mob */
#endif /* ANNOUNCE_MESSAGE_H_ */
