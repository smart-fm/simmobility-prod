/*
 * Message.hpp
 *
 *  Created on: May 22, 2013
 *      Author: zhang
 */

#ifndef MESSAGE_HPP_
#define MESSAGE_HPP_
#include "string"
#include <iostream>
#include<boost/shared_ptr.hpp>
namespace sim_mob {

namespace FMOD
{

class Message {
public:
	Message();
	virtual ~Message();
private:
	std::string originMsg;
	int messageID;
};

}

} /* namespace sim_mob */

typedef boost::shared_ptr<sim_mob::FMOD::Message> msg_ptr;

#endif /* MESSAGE_HPP_ */
