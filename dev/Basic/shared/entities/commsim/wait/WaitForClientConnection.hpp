/*
 * WaitForClientConnection.hpp
 *
 *  Created on: Jul 15, 2013
 *      Author: vahid
 *
 *      This Class is used by the broker to synchronize
 *      the connection of various types of clients who
 *      have to work together.
 */

#ifndef WAITFORCLIENTCONNECTION_HPP_
#define WAITFORCLIENTCONNECTION_HPP_
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>

namespace sim_mob {
class Broker;
class WaitForClientConnection {
	sim_mob::Broker & broker;
	bool wait_status;
	boost::condition_variable cond_var;
	boost::mutex	mutex_;
	void setWaitStatus(bool);
public:
	WaitForClientConnection(sim_mob::Broker &);
	sim_mob::Broker & getBroker() const;
	void thread_wait();
	void tryWait();
	void notify();
	virtual bool evaluate() = 0;
	bool isWaiting();
	virtual ~WaitForClientConnection();
};

} /* namespace sim_mob */
#endif /* WAITFORCLIENTCONNECTION_HPP_ */
