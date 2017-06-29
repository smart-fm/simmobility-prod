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


enum MobilityServiceControllerType : unsigned int
{
	SERVICE_CONTROLLER_UNKNOWN = 0b0000,
	SERVICE_CONTROLLER_GREEDY = 0b0001,
	SERVICE_CONTROLLER_SHARED = 0b0010,
	SERVICE_CONTROLLER_ON_HAIL = 0b0100,
	SERVICE_CONTROLLER_FRAZZOLI = 0b1000
};

const std::string fromMobilityServiceControllerTypetoString(MobilityServiceControllerType type);

class MobilityServiceController: public Agent
{
protected:

	explicit MobilityServiceController(const MutexStrategy& mtxStrat = sim_mob::MtxStrat_Buffered, unsigned int dummy=0,
			MobilityServiceControllerType type_ = SERVICE_CONTROLLER_UNKNOWN)
		: Agent(mtxStrat, -1), type(type_) {}

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

	const MobilityServiceControllerType type;

};

} /* namespace sim_mob */

#endif /* SHARED_ENTITIES_CONTROLLERS_MOBILITYSERVICECONTROLLER_HPP_ */
