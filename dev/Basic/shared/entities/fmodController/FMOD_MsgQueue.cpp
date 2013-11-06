//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * MessageQueue.cpp
 *
 *  Created on: May 22, 2013
 *      Author: zhang
 */

#include "FMOD_MsgQueue.hpp"

namespace sim_mob {

namespace FMOD
{

MessageList operator+(MessageList lst1, MessageList lst2)
{
	MessageList ret;

	ret = lst1+lst2;

	return ret;
}

FMOD_MsgQueue::FMOD_MsgQueue() {
	// TODO Auto-generated constructor stub

}

FMOD_MsgQueue::~FMOD_MsgQueue() {
	// TODO Auto-generated destructor stub
}

void FMOD_MsgQueue::PushMessage(std::string msg)
{
	boost::unique_lock< boost::mutex > lock(mutex);
	messages.push(msg);
	condition.notify_one();
}

bool FMOD_MsgQueue::PopMessage(std::string& msg)
{
	bool ret=false;
	boost::unique_lock< boost::mutex > lock(mutex);
	if(messages.size()>0)
	{
		msg = messages.front();
		ret = true;
		messages.pop();
	}
	return ret;
}

bool FMOD_MsgQueue::WaitPopMessage(std::string& msg, int seconds)
{
	bool ret = false;
	boost::system_time const timeout = boost::get_system_time() + boost::posix_time::milliseconds(seconds*1000);
	boost::unique_lock< boost::mutex > lock(mutex);
	while(messages.size()==0)
	{
		if( !condition.timed_wait(lock, timeout ) ){
			ret = false;
			break;
		}
	}

	if(messages.size() > 0 ){
		msg = messages.front();
		ret = true;
		messages.pop();
	}

	return ret;
}


MessageList FMOD_MsgQueue::ReadMessage()
{
	MessageList res;
	boost::unique_lock< boost::mutex > lock(mutex);
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
