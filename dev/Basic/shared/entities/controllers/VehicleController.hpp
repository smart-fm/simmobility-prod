/*
 * VehicleController.hpp
 *
 *  Created on: Feb 20, 2017
 *      Author: Akshay Padmanabha
 */

#ifndef VehicleController_HPP_
#define VehicleController_HPP_
#include <vector>

#include "entities/Agent.hpp"
#include "message/Message.hpp"

namespace sim_mob
{

class VehicleController : public Agent {
protected:
	explicit VehicleController(const MutexStrategy& mtxStrat = sim_mob::MtxStrat_Buffered, unsigned int freq = 0)
		: Agent(mtxStrat, -1), messageProcessFrequency(freq)
	{
	}

public:
	struct VehicleRequest
	{
		const timeslice currTick;
		const std::string personId;
		const unsigned int startNodeId;
		const unsigned int destinationNodeId;
	};

	enum MessageResult
	{
		MESSAGE_ERROR_BAD_NODE = 0,
		MESSAGE_ERROR_VEHICLES_UNAVAILABLE,
		MESSAGE_SUCCESS
	};

	virtual ~VehicleController();

	/**
	 * Adds a vehicle driver to the controller
	 * @param person Driver to be added
	 */
	void addVehicleDriver(Person* person);

	/**
	 * Removes the vehicle driver from the controller
	 * @param person Driver to be removed
	 */
	void removeVehicleDriver(Person* person);

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

	/** Store list of vehicle drivers */
	std::vector<Person*> vehicleDrivers;

	/** Store queue of requests */
	std::vector<VehicleRequest> requestQueue;

private:
	/**
	 * Performs the controller algorithm to assign vehicles to requests
	 */
	virtual std::vector<MessageResult> assignVehiclesToRequests() = 0;

	/** Keeps track of current local tick */
	unsigned int localTick = 0;

	/** Keeps track of how often to process messages */
	unsigned int messageProcessFrequency;
};
}
#endif /* VehicleController_HPP_ */


