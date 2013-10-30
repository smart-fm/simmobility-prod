/*
 * this file is inclusion place for custome Messgae classes
 * whose type will be used in defining templated handlers
 * This generic class is basically assumed to serve as a wrapper around a
 * data string with Json format.
 */

#ifndef MESSAGE_HPP_
#define MESSAGE_HPP_

#include <iostream>
#include<boost/shared_ptr.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <queue>
#include<json/json.h>
#include "logging/Log.hpp"

using namespace boost;

namespace sim_mob
{
/********************************************************************
 ************************* Message Class ****************************
 ********************************************************************
 */
//Forward Declaration
class Handler;
typedef boost::shared_ptr<sim_mob::Handler> hdlr_ptr;
namespace comm
{

typedef int MessageType;

//Base Message
template<class T>
class Message
{
	T data;
	hdlr_ptr handler;
public:
	Message();
	Message(T data_):data(data_){}
	hdlr_ptr supplyHandler(){
		return handler;;
	}
	void setHandler( hdlr_ptr handler_)
	{
		handler = handler_;
	}
	T& getData()
	{
		return data;
	}
};
}//namespace comm
//todo do something here. the following std::string is spoiling messge's templatization benefits
typedef Json::Value msg_data_t;
typedef sim_mob::comm::Message<msg_data_t> msg_t;
typedef boost::shared_ptr<msg_t> msg_ptr; //putting std::string here is c++ limitation(old standard). don't blame me!-vahid



/********************************************************************
 ************************* Message Handler **************************
 ********************************************************************
 */
//Forward Declaration
class Broker;

class Handler
{
public:
	virtual void handle(msg_ptr message_,Broker*) = 0;
};

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

}/* namespace comm */

}//namespace sim_mob

#endif
