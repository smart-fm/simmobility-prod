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
#include <iostream>

#include <boost/shared_ptr.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <json/json.h>

#include "logging/Log.hpp"

namespace sim_mob {

//Forward Declaration
class Broker;
class Handler;

namespace comm {

//typedef int MessageType;

///Base Message
class Message
{
	Json::Value data;
	boost::shared_ptr<sim_mob::Handler> handler;
public:
	Message();
	Message(const Json::Value& data_):data(data_){}
	boost::shared_ptr<sim_mob::Handler> supplyHandler(){
		return handler;
	}
	void setHandler( boost::shared_ptr<sim_mob::Handler> handler_)
	{
		handler = handler_;
	}
	Json::Value& getData()
	{
		return data;
	}
};
}//namespace comm




/********************************************************************
 ************************* Message Factory **************************
 ********************************************************************
 */

template <class RET,class MSG>
class MessageFactory {
public:
	virtual bool createMessage(MSG,RET ) = 0;
};

/********************************************************************
 ************************* Message Queue **************************
 ********************************************************************
 */

namespace comm{

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

template<class T>
MessageQueue<T>::~MessageQueue(){

}

//template<class T>
//bool MessageQueue<T>::ReadMessage(){
//	boost::unique_lock< boost::shared_mutex > lock(mutex);
//	return true;
//}

template<class T>
int MessageQueue<T>::size()
{
	boost::unique_lock< boost::shared_mutex > lock(mutex);
	return messageList.size();
}
template<class T>
bool MessageQueue<T>::isEmpty()
{
	boost::unique_lock< boost::shared_mutex > lock(mutex);
	return messageList.isEmpty();
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

}} //namespace sim_mob::comm
