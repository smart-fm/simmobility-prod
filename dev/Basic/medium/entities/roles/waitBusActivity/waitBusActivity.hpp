//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)


#pragma once

#include "entities/roles/Role.hpp"
#include "waitBusActivityFacets.hpp"

namespace sim_mob
{

class Agent;
class Person;
class BusStop;

namespace medium
{

class WaitBusActivityBehavior;
class WaitBusActivityMovement;
class BusDriver;

/**
 * A medium-term WaitBusActivity.
 * \author Seth N. Hetu
 * \author zhang huai peng
 */
class WaitBusActivity : public sim_mob::Role<Person_MT>, public UpdateWrapper<UpdateParams>
{
public:

	explicit WaitBusActivity(Person_MT *parent, 
							sim_mob::medium::WaitBusActivityBehavior* behavior = nullptr,
							sim_mob::medium::WaitBusActivityMovement* movement = nullptr,
							std::string roleName = std::string("WaitBusActivity_"),
							Role<Person_MT>::Type roleType = Role<Person_MT>::RL_WAITBUSACTITITY);

	virtual ~WaitBusActivity()
	{
	}

	virtual sim_mob::Role<Person_MT>* clone(Person_MT *parent) const;

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
	void increaseFailedBoardingTimes();

	/**
	 * message handler which provide a chance to handle message transfered from parent agent.
	 * @param type of the message.
	 * @param message data received.
	 */
	virtual void HandleParentMessage(messaging::Message::MessageType type,
									const messaging::Message& message);

	const BusStop* getStop() const
	{
		return stop;
	}

	const std::string getBusLines();

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

	void setWaitingTime(unsigned int time)
	{
		waitingTime = time;
	}

	const unsigned int getWaitingTime() const
	{
		return waitingTime;
	}

	void setFailedBoardingTimes(unsigned int times)
	{
		failedBoardingTimes = times;
	}

	const unsigned int getFailedBoardingTimes() const
	{
		return failedBoardingTimes;
	}

private:
	friend class WaitBusActivityBehavior;
	friend class WaitBusActivityMovement;

	/**record waiting time in the bus stop*/
	unsigned int waitingTime;
	/**pointer to waiting bus stop*/
	BusStop* stop;
	/**flag to indicate whether the waiting person has decided to board or not*/
	bool boardBus;
	/**failed boarding times*/
	unsigned int failedBoardingTimes;
};
}
}
