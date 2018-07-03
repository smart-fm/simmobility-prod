//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

namespace sim_mob {

class BrokerBase;


/**
 * A BrokerBlocker is used to tell the Broker if the right conditions have been met to continue simulation.
 * For example, we check if X Android emulators have connected, or if the ns-3 simulator has connected.
 * This check is efficient: it is only performed once per time tick, and the first time it passes a flag is
 * set that considers the check to pass for all subsequent time ticks.
 * Note that the Broker will still accept incoming connections while waiting for the BrokerBlockers to pass.
 */
class BrokerBlocker {
protected:
	BrokerBlocker(); ///<Only a subclass can instantiate this.
public:
	virtual ~BrokerBlocker();

	///If true, the condition this BrokerBlocker was created to check has passed, and will continue to pass for the rest of the simulation.
	bool pass(BrokerBase& broker);

protected:
	///Override this function to differentiate the behavior of multiple BrokerBase classes.
	virtual bool calculateWaitStatus(BrokerBase& broker) const = 0;
	bool passed; ///<Once passed, the check will always short-circuit as "pass". Sub-classes can reset this to re-test on the next time tick.

};

}

