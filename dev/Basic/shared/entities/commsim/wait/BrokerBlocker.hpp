/*
 * BrokerBlocker.hpp
 *
 *  Created on: Jul 15, 2013
 *      Author: vahid
 *
 *      This Class is used by the broker to synchronize
 *      the connection of various types of clients who
 *      have to work together.
 */

#ifndef BROKERBLOCKER_HPP_
#define BROKERBLOCKER_HPP_
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>

namespace sim_mob {
class Broker;
class BrokerBlocker {
	sim_mob::Broker & broker;
	bool wait_status;
	boost::mutex	mutex_;
protected:
	void setWaitStatus(bool);
public:
	BrokerBlocker(sim_mob::Broker &);
	sim_mob::Broker & getBroker() const;
	virtual bool calculateWaitStatus() = 0;
	bool isWaiting();
	virtual ~BrokerBlocker();
};

} /* namespace sim_mob */
#endif /* BROKERBLOCKER_HPP_ */
