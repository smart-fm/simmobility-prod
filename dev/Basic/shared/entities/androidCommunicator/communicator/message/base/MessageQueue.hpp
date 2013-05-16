/*
 * MessageQueue.h
 *
 *  Created on: May 13, 2013
 *      Author: vahid
 */

#ifndef MESSAGEQUEUE_H_
#define MESSAGEQUEUE_H_
#include <boost/thread/shared_mutex.hpp>
#include <queue>
#include "Message.hpp"
using namespace boost;

namespace sim_mob {
namespace comm{

class MessageQueue {
	typedef std::queue<msg_ptr> MessageList;
	boost::shared_mutex mutex;
public:
	MessageQueue();
	virtual ~MessageQueue();
	bool ReadMessage();
    void post(msg_ptr message);
    msg_ptr pop();
};

}/* namespace comm */
} /* namespace sim_mob */
#endif /* MESSAGEQUEUE_H_ */
