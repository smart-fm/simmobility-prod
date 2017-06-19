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
#include "entities/controllers/Rebalancer.hpp"

namespace sim_mob
{



class MobilityServiceController : public Agent {
protected:
	explicit MobilityServiceController(const MutexStrategy& mtxStrat = sim_mob::MtxStrat_Buffered, unsigned int computationPeriod = 0)
		: Agent(mtxStrat, -1), scheduleComputationPeriod(computationPeriod)
	{
		rebalancer = new SimpleRebalancer();
#ifndef NDEBUG
		isComputingSchedules = false;
#endif
	}

public:

	virtual ~MobilityServiceController();

	/**
	 * Signals are non-spatial in nature.
	 */
	bool isNonspatial();

	/*
	 * It returns the pointer to the driver closest to the node
	 */
	const Person* findClosestDriver(const Node* node) const;



protected:
	/**
	 * Inherited from base class agent to initialize parameters
	 */
	Entity::UpdateStatus frame_init(timeslice now);

	/**
	 * Inherited from base class to update this agent
	 */
	Entity::UpdateStatus frame_tick(timeslice now);

	/**
	 * Inherited from base class to output result
	 */
	void frame_output(timeslice now);

	/**
	 * Inherited from base class to handle message
	 */
    void HandleMessage(messaging::Message::MessageType type, const messaging::Message& message);

	/**
	 * Makes a vehicle driver unavailable to the controller
	 * @param person Driver to be removed
	 */
	void driverUnavailable(Person* person);

	/** Store list of subscribed drivers */
	std::vector<Person*> subscribedDrivers;

	/** Store list of available drivers */
	std::vector<Person*> availableDrivers;

	/** Store queue of requests */
	//TODO: It should be vector<const TripRequest>, but it does not compile in that case:
	// check why
	std::list<TripRequestMessage> requestQueue;

	void assignSchedule(const Person* driver, Schedule schedule);

	bool isCruising(Person* driver) const;
	const Node* getCurrentNode(Person* driver) const;
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
	double computeOptimalSchedule(const Node* initialPositon, const Schedule currentSchedule,
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
	double evaluateSchedule(const Node* initialPositon, const Schedule& schedule, double additionalDelayThreshold, double waitingTimeThreshold) const;


#ifndef NDEBUG
	bool isComputingSchedules; //true during computing schedules. Used for debug purposes
#endif

private:
	/**
	 * Subscribes a vehicle driver to the controller
	 * @param person Driver to be added
	 */
	void subscribeDriver(Person* person);

	/**
	 * Unsubscribes a vehicle driver from the controller
	 * @param person Driver to be removed
	 */
	void unsubscribeDriver(Person* person);

	/**
	 * Makes a vehicle driver available to the controller
	 * @param person Driver to be added
	 */
	void driverAvailable(Person* person);


	/** Keeps track of current local tick */
	unsigned int localTick = 0;

	/** Keeps track of how often to process messages */
	unsigned int scheduleComputationPeriod;

	Rebalancer* rebalancer;
};
}
#endif /* MobilityServiceController_HPP_ */

