//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "entities/mobilityServiceDriver/MobilityServiceDriver.hpp"
#include "entities/Person_MT.hpp"
#include "entities/roles/driver/Driver.hpp"
#include "OnHailDriverFacets.hpp"

namespace sim_mob
{
namespace medium
{

class OnHailDriver : public Driver, public MobilityServiceDriver
{
private:
	/**Pointer to the passenger that has been picked up by the driver*/
	Passenger *passenger;

	/**Stores the controllers that the driver is subscribed to*/
	std::vector<MobilityServiceController *> subscribedControllers;

protected:
	/**Pointer to the on hail driver's movement facet object*/
	OnHailDriverMovement *movement;

public:
	OnHailDriver(Person_MT *parent, const MutexStrategy &mtx,
	             OnHailDriverBehaviour *behaviour, OnHailDriverMovement *movement,
	             std::string roleName,
	             Role<Person_MT>::Type roleType = Role<Person_MT>::RL_ON_CALL_DRIVER);

	OnHailDriver(Person_MT *parent);

	virtual ~OnHailDriver();

	/**
	 * Clones the on call driver object
	 * @param person the person who will take on the cloned role
	 * @return the cloned on call driver role
	 */
	virtual Role<Person_MT> *clone(Person_MT *person) const;

	/**
	 * This method is a message handler.
	 * @param type type of the message
	 * @param msg the received message
	 */
	virtual void HandleParentMessage(messaging::Message::MessageType type, const messaging::Message &msg);

	/**
	 * The current node of the driver is the node which has most recently been crossed by the driver
	 * This method retrieves the node using the current segment from the path mover object.
	 * @return current node if available, else nullptr
	 */
	virtual const Node* getCurrentNode() const;

	/**
	 * @return vector of controllers that the driver has subscribed to
	 */
	virtual const std::vector<MobilityServiceController *>& getSubscribedControllers() const;

	/**
	 * Overrides the parent function. As on hail drivers do not follow a schedule, this method will
	 * throw a runtime error
	 * @return
	 */
	virtual Schedule getAssignedSchedule() const;

	/**
	 * There is no travel time collection for on hail drivers. This method does nothing
	 */
	virtual void collectTravelTime();

	/**
	 * @return the number of passengers in the vehicle. As on hail drivers can serve only 1 customer at a
	 * time, this method will only return a 0 or 1
	 */
	virtual unsigned long getPassengerCount() const;

	friend class OnHailDriverMovement;
};

}
}
