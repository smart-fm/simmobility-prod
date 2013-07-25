/*
 * WaitForClientConnection.cpp
 *
 *  Created on: Jul 15, 2013
 *      Author: vahid
 */

#include "WaitForClientConnection.hpp"
#include "entities/commsim/broker/Broker.hpp"

namespace sim_mob {

WaitForClientConnection::WaitForClientConnection(sim_mob::Broker & broker_): broker(broker_) {
	// TODO Auto-generated constructor stub
	wait_status = false;
	tryWait();
}



sim_mob::Broker & WaitForClientConnection::getBroker() const{
	return broker;
}

void WaitForClientConnection::thread_wait()
{
//	Print() << "inside WaitForClientConnection::thread_wait()" << std::endl;

	boost::unique_lock<boost::mutex> lock(mutex_);
	while(evaluate())
	{
		Print() << "WaitForClientConnection::thread_wait=>wait locked" << std::endl;
		wait_status = true;
		cond_var.wait(lock);
		Print() << "WaitForClientConnection::thread_wait=>wait released" << std::endl;

	}
//	Print() << "inside WaitForClientConnection::thread_wait()::NOT waiting" << std::endl;
	wait_status = false;
//	Print() << "inside WaitForClientConnection::thread_wait() dying" << std::endl;
}

void WaitForClientConnection::tryWait() {
	Print() << "inside WaitForClientConnection::tryWait()" << std::endl;
	if(isWaiting())
	{
		return;
	}
	boost::thread(&WaitForClientConnection::thread_wait, this);
}
void WaitForClientConnection::notify() {
//	Print() << "inside WaitForClientConnection::notify()" << std::endl;
	boost::unique_lock<boost::mutex> lock(mutex_);
	Print() << "WaitForClientConnection::notify()::notifying" << std::endl;
	cond_var.notify_one();
}

bool WaitForClientConnection::isWaiting() {
	boost::unique_lock<boost::mutex> lock(mutex_);
	return wait_status;
}

void WaitForClientConnection::setWaitStatus(bool value) {
	boost::unique_lock<boost::mutex> lock(mutex_);
	wait_status = value;
}

WaitForClientConnection::~WaitForClientConnection() {
	// TODO Auto-generated destructor stub
}

} /* namespace sim_mob */
