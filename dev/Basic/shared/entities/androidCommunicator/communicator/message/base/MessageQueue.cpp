/*
 * MessageQueue.cpp
 *
 *  Created on: May 13, 2013
 *      Author: vahid
 */

#include "MessageQueue.hpp"

namespace sim_mob {
namespace comm {


MessageQueue::~MessageQueue(){

}
bool MessageQueue::ReadMessage(){
	boost::unique_lock< boost::shared_mutex > lock(mutex);
	return true;
}
void MessageQueue::post(msg_ptr message){
	boost::unique_lock< boost::shared_mutex > lock(mutex);
}
void MessageQueue::pop(){
	boost::unique_lock< boost::shared_mutex > lock(mutex);
}

MessageQueue::MessageQueue() {
	// TODO Auto-generated constructor stub

}


} /* namespace comm */
} /* namespace sim_mob */
