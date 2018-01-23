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
	std::unordered_map<std::string, Passenger *> passengers;

	/**Stores the controllers that the driver is subscribed to*/
	std::vector<MobilityServiceController *> subscribedControllers;

	/**Indicates whether the driver is to be removed from the parking*/
	bool toBeRemovedFromParking;

	/**Indicates that the driver is exiting a parking node to go to a pickup node
	 * (or a drop off node if the parking and pick up nodes were the same)*/
	bool isExitingParking;

protected:
	/**Pointer to the on call driver's movement facet object*/
	OnCallDriverMovement *movement;

	/**Pointer to the on call driver's behaviour facet object*/
	OnCallDriverBehaviour *behaviour;

	/**Indicates whether the driver is waiting for an acknowledgement from the controller
	 * regarding successful unsubscription*/
	bool isWaitingForUnsubscribeAck;

	/**Indicates that the schedule has been set or updated by the controller*/
	bool isScheduleUpdated;

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

		void updateSchedule(const Schedule &updatedSchedule)
		{
			auto currIt = *currentItem;
			assignedSchedule = updatedSchedule;
			assignedSchedule.insert(assignedSchedule.begin(), currIt);
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

		bool isScheduleCompleted() const
		{
			return currentItem == assignedSchedule.end();
		}
	} driverSchedule;

	/**
	 * Marks the current schedule item as completed
	 */
	void scheduleItemCompleted();

	/**
	 * Checks if the current item in the schedule has been rescheduled
	 * @param updatedSchedule  the updated schedule
	 * @return true if the current item in the schedule has been rescheduled
	 */
	bool currentItemRescheduled(const Schedule &updatedSchedule);

	/**
	 * Sends the schedule received acknowledgement message
	 * @param success indicates whether the schedule can be performed
	 */
	void sendScheduleAckMessage(bool success);

	/**
	 * Sends the driver available message to the controllers
	 */
	void sendAvailableMessage();

	/**
	 * Sends the driver status update message
	 */
	void sendStatusMessage();

	/**
	 * Sends the message, indicating that the shift has ended and it has to wake up. This is
	 * required to wake up drivers who are parked when their shift ends
	 */
	void sendWakeUpShiftEndMsg();

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
	 * Message handler to provide a chance to handle message forwarded by the parent person.
	 * @param type type of the message.
	 * @param message the message containing the required data.
	 */
	virtual void HandleParentMessage(messaging::Message::MessageType type, const messaging::Message &message);

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
	void subscribeToOrIgnoreController(const SvcControllerMap& controllers, unsigned int controllerId);

	/**
	 * Picks up the passenger
	 */
	void pickupPassenger();

	/**
	 * Drops off the passenger
	 */
	void dropoffPassenger();

	/**
	 * Performs the tasks required to end the driver shift
	 */
	void endShift();

	const bool isToBeRemovedFromParking() const
	{
		return toBeRemovedFromParking;
	}

	void setToBeRemovedFromParking(bool value)
	{
		toBeRemovedFromParking = value;
	}

	friend class OnCallDriverMovement;
	friend class OnCallDriverBehaviour;
};

}
}