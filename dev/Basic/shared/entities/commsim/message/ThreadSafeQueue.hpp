//Copyright (c) 2014 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <queue>
#include <boost/thread/mutex.hpp>

#include "entities/commsim/message/MessageBase.hpp"

namespace sim_mob {

/**
 * Implements a simple queue that is thread-safe for all individual operations.
 * Use push() and pop() to add/remove members to/from the queue.
 * These operations are locked with a mutex.
 */
template<class T>
class ThreadSafeQueue {
public:
	///Push an item into the queue.
	void push(const T& item);

	///Pop an item off the queue and store it in res.
	///Returns true if an item was retrieved, false otherwise.
	bool pop(T& res);

	///Returns the size of the queue
	int size() const;

	///Remove all items in the queue (used for testing)
	void clear();

private:
	std::queue<T> messageList;
	boost::mutex mutex;
};

} //End sm4ns3


//////////////////////////////////////////////////////////
// Template implementation
//////////////////////////////////////////////////////////

template<class T>
void sim_mob::ThreadSafeQueue<T>::push(const T& item)
{
	boost::lock_guard<boost::mutex> lock(mutex);

	messageList.push(item);
}

template<class T>
bool sim_mob::ThreadSafeQueue<T>::pop(T &res)
{
	boost::lock_guard<boost::mutex> lock(mutex);

	if(!messageList.empty()) {
		res = messageList.front();
		messageList.pop();
		return true;
	}
	return false;
}


template<class T>
int sim_mob::ThreadSafeQueue<T>::size() const
{
	boost::lock_guard<boost::mutex> lock(mutex);

	return messageList.size();
}

template<class T>
void sim_mob::ThreadSafeQueue<T>::clear()
{
	boost::lock_guard<boost::mutex> lock(mutex);

	messageList = std::queue<T>();
}


