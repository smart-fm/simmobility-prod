/*
 * MobilityServiceController.hpp
 *
 *  Created on: Feb 20, 2017
 *      Author: Akshay Padmanabha
 */

#ifndef MobilityServiceController_HPP_
#define MobilityServiceController_HPP_
#include <vector>

#include "entities/Agent.hpp"
#include "message/Message.hpp"
#include "message/MobilityServiceControllerMessage.hpp"
#include "MobilityServiceController.hpp"
#include "entities/controllers/Rebalancer.hpp"

namespace sim_mob
{



class OnCallController : public MobilityServiceController
{
protected:
	explicit OnCallController(const MutexStrategy& mtxStrat = sim_mob::MtxStrat_Buffered, unsigned int computationPeriod=0)
		: MobilityServiceController(mtxStrat, computationPeriod), scheduleComputationPeriod(computationPeriod)
	{
		rebalancer = new SimpleRebalancer();
#ifndef NDEBUG
		isComputingSchedules = false;
#endif
	}

public:

	virtual ~OnCallController();


	/*
	 * It returns the pointer to the driver closest to the node
	 */
	virtual const Person* findClosestDriver(const Node* node) const;



protected:


	/**
	 * Inherited from base class to output result
	 */
	virtual void frame_output(timeslice now);

	/**
	 * Inherited from base class to handle message
	 */
	virtual void HandleMessage(messaging::Message::MessageType type, const messaging::Message& message);

	/**
	 * Makes a vehicle driver unavailable to the controller
	 * @param person Driver to be removed
	 */
	virtual void driverUnavailable(Person* person);

	/** Store list of available drivers */
	std::vector<Person*> availableDrivers;

	/** Store queue of requests */
	//TODO: It should be vector<const TripRequest>, but it does not compile in that case:
	// check why
	std::list<TripRequestMessage> requestQueue;

	virtual void assignSchedule(const Person* driver, Schedule schedule);

	virtual bool isCruising(Person* driver) const;
	virtual const Node* getCurrentNode(Person* driver) const;
	/**
	 * Performs the controller algorithm to assign vehicles to requests
	 */
	virtual void computeSchedules() = 0;

	/**
	 * Associates to each driver her current schedule
	 */
	std::map<const Person*, Schedule> driverSchedules;

	/**
	 * Computes a hypothetical schedule such that a driver located at a certain position can serve her current schedule
	 * as well as additional requests. The hypothetical schedule is written in newSchedule. The return value is the
	 * travel time.
	 */
	virtual double computeOptimalSchedule(const Node* initialPositon, const Schedule currentSchedule,
			const std::vector<TripRequestMessage>& additionalRequests,
			Schedule& newSchedule) const;

	/**
	 * Checks if the schedule is feasible, i.e. if:
	 * 1. no user is scheduled to be dropped off before being picked up
	 * 2. no user will experience an additional delay above the additionalDelayThreshold. The additonal delay is the
	 *    increase in her total time (including waiting and travel time
	 *    with respect to the case when she moves with a private vehicle
	 * 3. no user will experience a waiting time above waitingTimeThreshold
	 * If the schedule is feasible, it returns the travel time. Otherwise, it returns a negative number.
	 * The return value and the thresholds are expressed in ms.
	 */
	virtual double evaluateSchedule(const Node* initialPositon, const Schedule& schedule, double additionalDelayThreshold, double waitingTimeThreshold) const;

	/**
	 * Inherited from base class to update this agent
	 */
	Entity::UpdateStatus frame_tick(timeslice now);




#ifndef NDEBUG
	bool isComputingSchedules; //true during computing schedules. Used for debug purposes
#endif

private:
	/**
	 * Subscribes a vehicle driver to the controller
	 * @param person Driver to be added
	 */
	virtual void subscribeDriver(Person* person);

	/**
	 * Unsubscribes a vehicle driver from the controller
	 * @param person Driver to be removed
	 */
	virtual void unsubscribeDriver(Person* person);

	/**
	 * Makes a vehicle driver available to the controller
	 * @param person Driver to be added
	 */
	virtual void driverAvailable(Person* person);


	/** Keeps track of current local tick */
	unsigned int localTick = 0;

	/** Keeps track of how often to process messages */
	unsigned int scheduleComputationPeriod;

	Rebalancer* rebalancer;
};
}
#endif /* MobilityServiceController_HPP_ */

