//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)


#pragma once

#include "entities/roles/Role.hpp"
#include "waitBusActivityFacets.hpp"

namespace sim_mob {

class Agent;
class Person;
class BusStop;

namespace medium {

class WaitBusActivityBehavior;
class WaitBusActivityMovement;
class BusDriver;

enum Decision {
	NO_DECISION, BOARD_BUS, ALIGHT_BUS
};

/**
 * A medium-term WaitBusActivity.
 * \author Seth N. Hetu
 * \author zhang huai peng
 */
class WaitBusActivity: public sim_mob::Role {
public:

	explicit WaitBusActivity(Agent* parent, MutexStrategy mtxStrat,
			sim_mob::medium::WaitBusActivityBehavior* behavior = nullptr,
			sim_mob::medium::WaitBusActivityMovement* movement = nullptr);

	virtual ~WaitBusActivity() {
	}

	virtual sim_mob::Role* clone(sim_mob::Person* parent) const;

	virtual void make_frame_tick_params(timeslice now) {
		throw std::runtime_error(
				"make_frame_tick_params not implemented in WaitBusActivity.");
	}
	virtual std::vector<sim_mob::BufferedBase*> getSubscriptionParams() {
		throw std::runtime_error(
				"getSubscriptionParams not implemented in WaitBusActivity.");
	}

	Decision getDecision();
	void setDecision(Decision decision);
	void setStop(sim_mob::BusStop* busStop);

	/**
	 * increase waiting time every frame tick.
	 *
	 * @param incrementMs is accumulation time in milli seconds
	 */
	void increaseWaitingTime(unsigned int incrementMs);

	/**
	 * make a decision for boarding.
	 *
	 * @param driver is which just come to the bus stop
	 */
	void makeBoardingDecision(BusDriver* driver);

	/**
	 * message handler which provide a chance to handle message transfered from parent agent.
	 * @param type of the message.
	 * @param message data received.
	 */
	virtual void HandleParentMessage(messaging::Message::MessageType type,
			const messaging::Message& message);


private:
	friend class WaitBusActivityBehavior;
	friend class WaitBusActivityMovement;

	/**record waiting time in the bus stop*/
	unsigned int waitingTime;
	/**pointer to waiting bus stop*/
	BusStop* stop;
	/**decision result*/
	Decision decision;
};
}
}
