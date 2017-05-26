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

namespace sim_mob
{

class MobilityServiceController : public Agent {
protected:
	explicit MobilityServiceController(const MutexStrategy& mtxStrat = sim_mob::MtxStrat_Buffered, unsigned int computationPeriod = 0)
		: Agent(mtxStrat, -1), scheduleComputationPeriod(computationPeriod)
	{
	}

public:
	struct TripRequest
	{
		const timeslice currTick;
		const std::string personId;
		const unsigned int startNodeId;
		const unsigned int destinationNodeId;
		const unsigned int extraTripTimeThreshold;
	};

	enum MessageResult
	{
		MESSAGE_ERROR_BAD_NODE = 0,
		MESSAGE_ERROR_VEHICLES_UNAVAILABLE,
		MESSAGE_SUCCESS
	};

	virtual ~MobilityServiceController();

	/**
	 * Signals are non-spatial in nature.
	 */
	bool isNonspatial();

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
	std::vector<TripRequest> requestQueue;

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

	/**
	 * Performs the controller algorithm to assign vehicles to requests
	 */
	virtual std::vector<MessageResult> computeSchedules() = 0;

	/** Keeps track of current local tick */
	unsigned int localTick = 0;

	/** Keeps track of how often to process messages */
	unsigned int scheduleComputationPeriod;
};
}
#endif /* MobilityServiceController_HPP_ */

