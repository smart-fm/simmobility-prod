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

void MessageQueue::PushMessage(std::string msg)
{
	boost::unique_lock< boost::shared_mutex > lock(mutex);
	messages.push(msg);
}

bool MessageQueue::PopMessage(std::string& msg)
{
	bool ret=false;
	boost::unique_lock< boost::shared_mutex > lock(mutex);
	if(messages.size()>0)
	{
		msg = messages.front();
		ret = true;
		messages.pop();
	}
	return ret;
}

MessageList MessageQueue::ReadMessage()
{
	MessageList res;
	boost::unique_lock< boost::shared_mutex > lock(mutex);
	while(messages.size()>0)
	{
		std::string msg = messages.front();
		res.push(msg);
		messages.pop();
	}
	return res;
}

}

} /* namespace sim_mob */
