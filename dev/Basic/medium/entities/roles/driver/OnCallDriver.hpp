//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "Driver.hpp"
#include "entities/mobilityServiceDriver/MobilityServiceDriver.hpp"
#include "entities/roles/passenger/Passenger.hpp"
#include "OnCallDriverFacets.hpp"

namespace sim_mob
{
namespace medium
{

class OnCallDriver : public Driver, public MobilityServiceDriver
{
private:
	/**Stores the passengers that are in the vehicle*/
	std::set<Passenger *> passengers;

	/**Stores the controllers that the driver is subscribed to*/
	std::vector<MobilityServiceController *> subscribedControllers;

	/**Stores the schedule currently assigned to the driver*/
	Schedule assignedSchedule;

protected:
	/**Pointer to the on hail driver's movement facet object*/
	OnCallDriverMovement *movement;

public:
	OnCallDriver(Person_MT *parent, const MutexStrategy &mtx,
	             OnCallDriverBehaviour *behaviour, OnCallDriverMovement *movement,
	             std::string roleName,
	             Role<Person_MT>::Type = Role<Person_MT>::RL_ON_CALL_DRIVER);

	OnCallDriver(Person_MT *parent);

	virtual ~OnCallDriver();

	/**
	 * Clones the on call driver object
	 * @param person the person who will take on the cloned role
	 * @return the cloned on call driver role
	 */
	virtual Role<Person_MT> *clone(Person_MT *person) const;

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
	 * Overrides the parent function. As on call drivers follow a schedule, this method will
	 * return the current schedule assigned to the driver
	 * @return
	 */
	virtual Schedule getAssignedSchedule() const;

	/**
	 * @return the number of passengers in the vehicle. As on hail drivers can serve only 1 customer at a
	 * time, this method will only return a 0 or 1
	 */
	virtual unsigned long getPassengerCount() const;

	friend class OnCallDriverMovement;
};

}
}