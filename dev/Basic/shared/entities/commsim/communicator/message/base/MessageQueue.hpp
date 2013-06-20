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

template<class T>
class MessageQueue {
	std::queue<T> messageList;
	boost::shared_mutex mutex;
public:
	MessageQueue();
	virtual ~MessageQueue();
	bool ReadMessage();
    void post(T message);
    bool pop(T&);
};

template<class T>
MessageQueue<T>::~MessageQueue(){

}

template<class T>
bool MessageQueue<T>::ReadMessage(){
	boost::unique_lock< boost::shared_mutex > lock(mutex);
	return true;
}

template<class T>
void MessageQueue<T>::post(T message){
	boost::unique_lock< boost::shared_mutex > lock(mutex);
	messageList.push(message);
}

template<class T>
bool MessageQueue<T>::pop(T &t ){
	boost::unique_lock< boost::shared_mutex > lock(mutex);
	if(messageList.empty())
	{
		return false;
	}
	t = messageList.front();
	messageList.pop();
	return true;
}


template<class T>
MessageQueue<T>::MessageQueue() {
	// TODO Auto-generated constructor stub

}

}/* namespace comm */
} /* namespace sim_mob */
#endif /* MESSAGEQUEUE_H_ */
