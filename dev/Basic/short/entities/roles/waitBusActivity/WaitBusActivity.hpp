//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "entities/roles/Role.hpp"
#include "entities/Person_ST.hpp"
#include "geospatial/network/PT_Stop.hpp"

namespace sim_mob
{

class BusDriver;
class WaitBusActivityBehavior;
class WaitBusActivityMovement;

class WaitBusActivity: public Role<Person_ST>, public UpdateWrapper<UpdateParams>
{
private:
	/**Records the waiting time (in milliseconds) at the bus stop*/
	unsigned int waitingTime;
	
	/**Indicates whether the waiting person has decided to board*/
	bool decidedToBoardBus;
	
	/**Indicates whether the person has boarded the bus*/
	bool hasBoardedBus;
	
	/**Counts the number of times the person failed to board*/
	unsigned int failedToBoardCount;
	
	/**The driver of the bus that the waiting person has decided to board*/
	const BusDriver *busDriver;
	
public:
	explicit WaitBusActivity(Person_ST *parent, WaitBusActivityBehavior *behavior = nullptr,
			WaitBusActivityMovement *movement = nullptr, std::string roleName = std::string("WaitBusActivity_"),
			Role<Person_ST>::Type roleType = Role<Person_ST>::RL_WAITBUSACTIVITY);

	virtual ~WaitBusActivity();

	virtual Role<Person_ST>* clone(Person_ST *parent) const;

	virtual void make_frame_tick_params(timeslice now);

	virtual std::vector<BufferedBase *> getSubscriptionParams();

	/**
	 * Collects the travel time for the current role
	 */
	virtual void collectTravelTime();

	/**
	 * Increments the waiting time every frame tick.
	 *
	 * @param incrementMs the accumulated time in milli-seconds
	 */
	void increaseWaitingTime(unsigned int incrementMs);

	/**
	 * Makes the decision on whether or not to board the arriving bus
	 *
	 * @param The driver of the arriving bus
	 */
	void makeBoardingDecision(const BusDriver *driver);

	/**
	 * Increments the denied boarding count 
	 */
	void incrementDeniedBoardingCount();
	
	/**
	 * Stores the waiting time
	 * 
	 * @param waitingActivity is pointer to the waiting people
	 */
	void storeWaitingTime(const std::string &busLine);

	/**
	 * Message handler to handle messages transfered from the parent agent.
	 * 
	 * @param type of the message.
	 * @param message data received.
	 */
	virtual void HandleParentMessage(messaging::Message::MessageType type, const messaging::Message& message);
	
	const std::string getBusLines() const
	{
		return parent->currSubTrip->getBusLineID();
	}

	bool hasDecidedToBoardBus() const
	{
		return decidedToBoardBus;
	}

	void setDecidedToBoardBus(bool boardBus)
	{
		this->decidedToBoardBus = boardBus;
	}

	unsigned int getWaitingTime() const
	{
		return waitingTime;
	}

	unsigned int getDeniedBoardingCount() const
	{
		return failedToBoardCount;
	}
	
	friend class WaitBusActivityBehavior;
	friend class WaitBusActivityMovement;
};

}
