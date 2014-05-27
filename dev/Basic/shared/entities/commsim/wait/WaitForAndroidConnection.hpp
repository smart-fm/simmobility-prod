//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)


#pragma once

#include "BrokerBlocker.hpp"

namespace sim_mob {

class WaitForAndroidConnection: public sim_mob::BrokerBlocker {
public:
	WaitForAndroidConnection();
	virtual ~WaitForAndroidConnection();

	void reset(unsigned int numClients);
	virtual bool calculateWaitStatus(BrokerBase& broker) const;

private:
	unsigned int numClients;
};

}
