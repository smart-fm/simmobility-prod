//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "entities/roles/Role.hpp"
#include "geospatial/BusStop.hpp"
#include "entities/Person.hpp"
#include "WaitBusActivityFacets.hpp"

namespace sim_mob
{
namespace medium
{
class BusDriver;

/**
 * A medium-term WaitBusActivity.
 * \author Seth N. Hetu
 * \author zhang huai peng
 */
class WaitBusActivity: public sim_mob::Role, public UpdateWrapper<UpdateParams>
{
public:
	explicit WaitBusActivity(Person* parent, MutexStrategy mtxStrat,
			sim_mob::medium::WaitBusActivityBehavior* behavior = nullptr,
			sim_mob::medium::WaitBusActivityMovement* movement = nullptr,
			std::string roleName = std::string("WaitBusActivity_"),
			Role::type roleType = Role::RL_WAITBUSACTITITY);

	virtual ~WaitBusActivity();

	virtual sim_mob::Role* clone(sim_mob::Person* parent) const;

	virtual void make_frame_tick_params(timeslice now);

	virtual std::vector<sim_mob::BufferedBase*> getSubscriptionParams();

	/**
	 * collect travel time for current role
	 */
	virtual void collectTravelTime();

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
	 * increase failed boarding times
	 */
	void incrementDeniedBoardingCount();

	/**
	 * message handler which provide a chance to handle message transfered from parent agent.
	 * @param type of the message.
	 * @param message data received.
	 */
	virtual void HandleParentMessage(messaging::Message::MessageType type, const messaging::Message& message);

	const BusStop* getStop() const
	{
		return stop;
	}

	const std::string& getBusLines() const;

	void setStop(sim_mob::BusStop* busStop)
	{
		stop = busStop;
	}

	bool canBoardBus() const
	{
		return boardBus;
	}

	void setBoardBus(bool boardBus)
	{
		this->boardBus = boardBus;
	}

	unsigned int getWaitingTime() const
	{
		return waitingTime;
	}

	unsigned int getDeniedBoardingCount() const
	{
		return failedToBoardCount;
	}

private:
	friend class WaitBusActivityBehavior;
	friend class WaitBusActivityMovement;

	/**record waiting time (in milliseconds) in the bus stop*/
	unsigned int waitingTime;
	/**pointer to waiting bus stop*/
	BusStop* stop;
	/**flag to indicate whether the waiting person has decided to board or not*/
	bool boardBus;
	/**failed boarding times*/
	unsigned int failedToBoardCount;
};
}
}
