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
#include "entities/controllers/Rebalancer.hpp"
#include "message/MessageBus.hpp"
#include "logging/ControllerLog.hpp"
#include "entities/mobilityServiceDriver/MobilityServiceDriver.hpp"


namespace sim_mob
{


enum MobilityServiceControllerType : unsigned int
{
	SERVICE_CONTROLLER_UNKNOWN = 0b0000,
	SERVICE_CONTROLLER_GREEDY = 0b0001,
	SERVICE_CONTROLLER_SHARED = 0b0010,
	SERVICE_CONTROLLER_ON_HAIL = 0b0100,
	SERVICE_CONTROLLER_FRAZZOLI = 0b1000,
	SERVICE_CONTROLLER_INCREMENTAL = 0b10000,
	SERVICE_CONTROLLER_PROXIMITY = 0b100000
};

const std::string toString(const MobilityServiceControllerType type);

/**
 * Raises an exception if the controller type is unrecognized. Does nothing otherwise
 */
void consistencyChecks(const MobilityServiceControllerType type);


//const std::string fromMobilityServiceControllerTypetoString(MobilityServiceControllerType type);

class MobilityServiceController : public Agent
{
protected:
	// We use explicit to avoid accidentally passing an integer instead of a MobilityServiceControllerType
	// (see https://stackoverflow.com/a/121163/2110769)
	// The constructor is protected to avoid instantiating an OnCallController directly, since it is conceptually abstract
	explicit MobilityServiceController(const MutexStrategy &mtxStrat,
	                                   MobilityServiceControllerType type_, unsigned id_)
			: Agent(mtxStrat, id_), controllerServiceType(type_), controllerId(id_)
	{
#ifndef NDEBUG
		sim_mob::consistencyChecks(type_);
#endif
		ControllerLog() << "MobilityServiceController instantiated, type:" << sim_mob::toString(controllerServiceType)
		                << ", id:" << controllerId << ", pointer:" << this << std::endl;

	}

	virtual void HandleMessage(messaging::Message::MessageType type, const messaging::Message &message);

	/** Store list of subscribed drivers */
	std::vector<Person *> subscribedDrivers;


	/**
	 * Subscribes a vehicle driver to the controller
	 * @param person Driver to be added
	 */
	virtual void subscribeDriver(Person *person);

	/**
	 * Unsubscribes a vehicle driver from the controller
	 * @param person Driver to be removed
	 */
	virtual void unsubscribeDriver(Person *person);

	/**
	 * Marks the schedule assigned to the driver as complete and
	 * unsubscribes the driver
	 * @param person the driver who has completed the scheule
	 */
	virtual void onDriverShiftEnd(Person *person);

	/**
	 * Updates the controller's copy of the driver schedule
	 * @param person the driver
	 */
	//aa!!: This only concerns the OnCall controller and should be removed from here
	virtual void onDriverScheduleStatus(Person *person);

	const MobilityServiceControllerType controllerServiceType;

	const unsigned controllerId;


public:

	static const unsigned toleratedExtraTime; //seconds

	/**
	 * The maximum that a user is willing to wait before being picked up
	 */
	static const double maxWaitingTime; // seconds

	/**
	 * Maximum number of people we can put together in a single vehicle
	 */
	static const unsigned maxAggregatedRequests;


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

	/**
	 * Overrides the correspondent function of MessageHandler
	 */
	virtual void onRegistrationOnTheMessageBus() const;


	void consistencyChecks() const;

	MobilityServiceControllerType getServiceType() const;

	unsigned getControllerId() const;

	const std::string toString() const;

	/**
	 * Overrides the parent function
	 */
	virtual void setToBeRemoved();
};


} /* namespace sim_mob */

#endif /* SHARED_ENTITIES_CONTROLLERS_MOBILITYSERVICECONTROLLER_HPP_ */
