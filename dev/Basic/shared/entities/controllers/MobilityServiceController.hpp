/*
 * MobilityServiceController.h
 *
 *  Created on: 21 Jun 2017
 *      Author: araldo
 */

#ifndef SHARED_ENTITIES_CONTROLLERS_MOBILITYSERVICECONTROLLER_HPP_
#define SHARED_ENTITIES_CONTROLLERS_MOBILITYSERVICECONTROLLER_HPP_

#include "entities/Agent.hpp"
#include "message/Message.hpp"
#include "message/MobilityServiceControllerMessage.hpp"
#include "MobilityServiceController.hpp"
#include "entities/controllers/Rebalancer.hpp"
#include "message/MessageBus.hpp"


namespace sim_mob {

class MobilityServiceController: public Agent
{
protected:

	explicit MobilityServiceController(const MutexStrategy& mtxStrat = sim_mob::MtxStrat_Buffered, unsigned int dummy=0)
		: Agent(mtxStrat, -1) {}

	virtual void HandleMessage(messaging::Message::MessageType type, const messaging::Message& message);

	/** Store list of subscribed drivers */
	std::vector<Person*> subscribedDrivers;


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

public:
	virtual ~MobilityServiceController();

	/**
	 * Inherited from base class agent to initialize parameters
	 */
	virtual Entity::UpdateStatus frame_init(timeslice now);

	/**
	 * Inherited from base class to update this agent
	 */
	virtual Entity::UpdateStatus frame_tick(timeslice now);


	/**
	 * Inherited.
	 */
	virtual bool isNonspatial();

	/**
	 * Inherited from base class to output result
	 */
	void frame_output(timeslice now);

};

} /* namespace sim_mob */

#endif /* SHARED_ENTITIES_CONTROLLERS_MOBILITYSERVICECONTROLLER_HPP_ */
