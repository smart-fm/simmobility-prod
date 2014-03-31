//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>

namespace sim_mob {

class Broker;

class BrokerBlocker {
public:
	BrokerBlocker(sim_mob::Broker& broker);
	virtual ~BrokerBlocker();

	sim_mob::Broker& getBroker() const;
	virtual bool calculateWaitStatus() = 0;
	bool isWaiting();

protected:
	void setWaitStatus(bool);

private:
	sim_mob::Broker & broker;
	bool wait_status;
	boost::mutex	mutex_;

};

}

