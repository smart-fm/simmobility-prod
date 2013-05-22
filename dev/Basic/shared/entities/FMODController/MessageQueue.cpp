/*
 * MessageQueue.cpp
 *
 *  Created on: May 22, 2013
 *      Author: zhang
 */

#include "MessageQueue.hpp"

namespace sim_mob {

namespace FMOD
{

MessageQueue::MessageQueue() {
	// TODO Auto-generated constructor stub

}

MessageQueue::~MessageQueue() {
	// TODO Auto-generated destructor stub
}

void MessageQueue::PushMessage(msg_ptr msg)
{
	boost::unique_lock< boost::shared_mutex > lock(mutex);
	messages.push(msg);
}

MessageList MessageQueue::ReadMessage()
{
	MessageList res;
	boost::unique_lock< boost::shared_mutex > lock(mutex);
	while(messages.size()>0)
	{
		msg_ptr msg = messages.front();
		res.push(msg);
		messages.pop();
	}
	return res;
}

}

} /* namespace sim_mob */
