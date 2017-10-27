//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "Driver.hpp"
#include "entities/controllers/MobilityServiceController.hpp"
#include "entities/controllers/MobilityServiceControllerManager.hpp"
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

protected:
	/**Pointer to the on call driver's movement facet object*/
	OnCallDriverMovement *movement;

	/**Pointer to the on call driver's behaviour facet object*/
	OnCallDriverBehaviour *behaviour;

	/**Wrapper for the schedule that has been given by the controller. */
	struct DriverSchedule
	{
	private:
		/**Stores the schedule currently assigned to the driver*/
		Schedule assignedSchedule;

		/**Points to the current schedule item being performed*/
		Schedule::const_iterator currentItem;

		/**Points to the next schedule item to be performed*/
		Schedule::const_iterator nextItem;

	public:
		void setSchedule(const Schedule &newSchedule)
		{
			assignedSchedule = newSchedule;
			currentItem = assignedSchedule.begin();
			nextItem = currentItem + 1;
		}

		const Schedule& getSchedule() const
		{
			return assignedSchedule;
		}

		Schedule::const_iterator getCurrScheduleItem() const
		{
			return currentItem;
		}

		Schedule::const_iterator getNextScheduleItem() const
		{
			return nextItem;
		}

		void itemCompleted()
		{
			++currentItem;
			++nextItem;
		}
	} driverSchedule;

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
	 * @return the number of passengers in the vehicle.
	 */
	virtual unsigned long getPassengerCount() const;

	/**
	 * Export service driver
	 * @return exporting result
	 */
	virtual const MobilityServiceDriver *exportServiceDriver() const;

	/**
	 * Checks if the driver is supposed to subscribe to the given controller type. If so, it subscribes it to all
	 * controllers of that it, else does nothing.
	 * @param controllers map of controllers
	 * @param type the type of controller to be subscribed
	 */
	void subscribeToOrIgnoreController(const SvcControllerMap& controllers, MobilityServiceControllerType type);

	/**
	 * Performs the tasks required to end the driver shift
	 */
	void endShift();

	friend class OnCallDriverMovement;
	friend class OnCallDriverBehaviour;
};

}
}