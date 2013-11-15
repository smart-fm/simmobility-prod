//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * this file is inclusion place for custome Messgae classes
 * whose type will be used in defining templated handlers
 * This generic class is basically assumed to serve as a wrapper around a
 * data string with Json format.
 */

#pragma once

#include <queue>

#include <boost/thread/shared_mutex.hpp>

namespace sim_mob {
namespace comm {

///MessageQueue (no documentation provided).
template<class T>
class MessageQueue {
	std::queue<T> messageList;
	boost::shared_mutex mutex;
public:
	MessageQueue();
	virtual ~MessageQueue();
	int size();
	bool isEmpty();
    void post(T message);
    bool pop(T&);
};

}} //End namespace sim_mob::comm


/////////////////////////////////////////////////////////////
// Message Queue template implementation
/////////////////////////////////////////////////////////////


template<class T>
sim_mob::comm::MessageQueue<T>::MessageQueue()
{}

template<class T>
sim_mob::comm::MessageQueue<T>::~MessageQueue()
{}

template<class T>
int sim_mob::comm::MessageQueue<T>::size()
{
	boost::unique_lock< boost::shared_mutex > lock(mutex);
	return messageList.size();
}

template<class T>
bool sim_mob::comm::MessageQueue<T>::isEmpty()
{
	boost::unique_lock< boost::shared_mutex > lock(mutex);
	return messageList.isEmpty();
}

template<class T>
void sim_mob::comm::MessageQueue<T>::post(T message){
	boost::unique_lock< boost::shared_mutex > lock(mutex);
	messageList.push(message);
}

template<class T>
bool sim_mob::comm::MessageQueue<T>::pop(T &t ){
	boost::unique_lock< boost::shared_mutex > lock(mutex);
	if(messageList.empty())
	{
		return false;
	}
	t = messageList.front();
	messageList.pop();
	return true;
}
