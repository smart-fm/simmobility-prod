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
#include "Message.hpp"
namespace sim_mob {

namespace FMOD
{

typedef std::queue<msg_ptr> MessageList;

class MessageQueue {
public:
	MessageQueue();
	virtual ~MessageQueue();

private:
	boost::shared_mutex mutex;
	MessageList messages;

public:
	void PushMessage(msg_ptr msg);
	MessageList ReadMessage();

};

}

} /* namespace sim_mob */
#endif /* MESSAGEQUEUE_HPP_ */
