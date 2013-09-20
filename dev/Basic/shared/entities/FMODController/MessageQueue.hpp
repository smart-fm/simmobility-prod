//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * MessageQueue.hpp
 *
 *  Created on: May 22, 2013
 *      Author: zhang
 */

#ifndef MESSAGEQUEUE_HPP_
#define MESSAGEQUEUE_HPP_

#include "boost/thread/shared_mutex.hpp"
#include "queue"

namespace sim_mob {

namespace FMOD
{

typedef std::queue<std::string> MessageList;


MessageList operator+(MessageList lst1, MessageList lst2);

class MessageQueue {
public:
	MessageQueue();
	virtual ~MessageQueue();

private:
	boost::mutex mutex;
	boost::condition_variable condition;
	MessageList messages;

public:
	void PushMessage(std::string msg);
	bool PopMessage(std::string& msg);
	bool WaitPopMessage(std::string& msg, int seconds);
	MessageList ReadMessage();

};

}

} /* namespace sim_mob */
#endif /* MESSAGEQUEUE_HPP_ */
