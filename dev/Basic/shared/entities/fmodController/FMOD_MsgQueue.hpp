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

/**
  * buffer FMOD message
  */
class FMOD_MsgQueue {
public:
	FMOD_MsgQueue();
	virtual ~FMOD_MsgQueue();

private:
	boost::mutex mutex;
	boost::condition_variable condition;
	MessageList messages;

public:

    /**
      * push a message to this storage temperally.
      * @param msg is to store message content
      * @return void.
      */
	void PushMessage(std::string msg);

    /**
      * pop up the first message from this storage.
      * @param msg is to store message content
      * @return void.
      */
	bool PopMessage(std::string& msg);

    /**
      * waiting a message in blocking mode until a message is received.
      * @param msg is to store message content
      * @timeoutSeconds is to assign timeout for blocking
      * @return true if retrieve a message successfully. otherwise false.
      */
	bool WaitPopMessage(std::string& msg, int timeoutSeconds);

    /**
      * read all messages at one time.
      * @return a list of FMOD message.
      */
	MessageList ReadMessage();

};

}

} /* namespace sim_mob */
#endif /* MESSAGEQUEUE_HPP_ */
